#pragma once

struct Point {
	float x, y, z;

	Point(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
	Point(float xx) : x(xx), y(xx), z(xx) {}
	Point() : x(0.0f), y(0.0f), z(0.0f) {}

	Point operator/(float p)
	{
		return Point(x / p, y / p, z / p);
	}
	Point operator*(float p)
	{
		return Point(x * p, y * p, z * p);
	}
	Point operator+(Point p)
	{
		return Point(x + p.x, y + p.y, z + p.z);
	}
};