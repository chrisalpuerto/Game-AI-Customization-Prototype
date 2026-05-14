#include "myPoint.h"

myPoint::myPoint(float x, float y, float z)
{
	tuple[0] = x;
	tuple[1] = y;
	tuple[2] = z;
}

myPoint::myPoint()
{
	tuple[0] = tuple[1] = tuple[2] = 0;
}
myPoint::~myPoint() {}

myPoint myPoint::AddVectorToPoint(Vector* vec_in)
{
	myPoint pt_out;
	for (int i = 0; i < 3; i++)
	{
		pt_out.tuple[i] = tuple[i] + vec_in->tuple[i];
	}
	return pt_out;
}
myPoint myPoint::SubtractVectorFromPoint(Vector* vec_in)
{
	myPoint pt_out;
	for (int i = 0; i < 3; i++)
	{
		pt_out.tuple[i] = tuple[i] - vec_in->tuple[i];
	}
	return pt_out;
}
Vector myPoint::SubtractPointFromPoint(myPoint* pt_in)
{
	Vector vec_out;
	for (int i = 0; i < 3; i++)
	{
		vec_out.tuple[i] = tuple[i] - pt_in->tuple[i];
	}
	return vec_out;
}

void myPoint::toggle()
{
	on = !on;
}

bool myPoint::get_on()
{
	return on;
}