#include "SharedColosBuffer.h"
#include <iostream>

SharedColosBuffer::SharedColosBuffer(Point size) :
	size(size)
{


	buf_size = (size.x * size.y * size.z * 3 + 1) * sizeof(unsigned char);
	colors = (unsigned char (*)[]) malloc(buf_size);

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, buf_size, TEXT("VirtualCubeSHMemmory"));
	if (hMapFile == NULL)
		throw std::exception("Was not able to create or connect to shared memmory");

	DWORD error = GetLastError();

	pBuf = (LPTSTR)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, buf_size);
	if (pBuf == NULL) {
		CloseHandle(hMapFile);
		throw std::exception("Was not able to map shared file");
	}

	fbyte = (char*)pBuf;

	if (error != ERROR_ALREADY_EXISTS) {
		memset(pBuf, 20, buf_size);
		*fbyte = 1;
	}
}

SharedColosBuffer::~SharedColosBuffer()
{
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
}

void SharedColosBuffer::readMemmory()
{
	if (*fbyte == 1) {
		*fbyte = 2;
		CopyMemory(colors, pBuf, buf_size);
		*fbyte = 0;
	}
}

Point SharedColosBuffer::getAt(int x, int y, int z)
{
	int flat = (size.z * size.y * z + size.x * y + x) * 3 + 1;
	return Point((*colors)[flat] / 255.0f, (*colors)[flat + 1] / 255.0f, (*colors)[flat + 2] / 255.0f);
}

