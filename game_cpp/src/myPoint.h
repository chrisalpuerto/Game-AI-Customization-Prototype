#pragma once
#include "Vector.h"
#include <cmath>
class myPoint
{
public:
	myPoint(float x, float y, float z);
	myPoint();
	~myPoint();


	float tuple[3]; //(x, y, z)

	bool on = false;
	void toggle();
	bool get_on();

	myPoint AddVectorToPoint(Vector* vec_in);
	myPoint SubtractVectorFromPoint(Vector* vec_in);
	Vector SubtractPointFromPoint(myPoint* pt_in);

	bool operator != (myPoint const& obj) {

		for (int i = 0; i < 3; i++)
		{
			if (int(tuple[i]*10) != int(obj.tuple[i]*10))
			{
				return true;
			}			
		}
		return false;
	}

	bool operator == (myPoint const& obj) {

		for (int i = 0; i < 3; i++)
		{
			if (int(tuple[i]*10) != int(obj.tuple[i]*10))
			{
				return false;
			}
		}
		return true;
	}

	Vector operator - (myPoint const& obj) {

		Vector vec_out;
		for (int i = 0; i < 3; i++)
		{
			vec_out.tuple[i] = tuple[i] - obj.tuple[i];
		}
		return vec_out;
	}

	myPoint operator - (Vector const& obj) {

		myPoint pt_out;
		for (int i = 0; i < 3; i++)
		{
			pt_out.tuple[i] = tuple[i] - obj.tuple[i];
		}
		return pt_out;
	}

	myPoint operator + (Vector const& obj) {

		myPoint pt_out;
		for (int i = 0; i < 3; i++)
		{
			pt_out.tuple[i] = tuple[i] + obj.tuple[i];
		}
		return pt_out;
	}

};

/*
Variables:
		num tuple[3]; //(x,y,z)

	Operators:
		myPoint AddVectorToPoint(Vector);
		myPoint SubtractVectorFromPoint(Vector);
		Vector SubtractPointFromPoint(myPoint);

	Function:
		//later this will call a function from a graphics API, but for now
		//this should just be printing the points coordinates to the screen
		drawPoint;
*/