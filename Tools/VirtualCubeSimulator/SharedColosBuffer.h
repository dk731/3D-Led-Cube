#pragma once
#include <string>
#include "Point.h"
#include <vector>
#include <windows.h>

class SharedColosBuffer
{

public:
	int buf_size;
	Point size;
	HANDLE hMapFile;
	void* pBuf;
	char* fbyte;
	unsigned char (*colors)[];
	
	

	SharedColosBuffer(Point size);
	virtual ~SharedColosBuffer();

	void readMemmory();
	Point getAt(int x, int y, int z);

};

