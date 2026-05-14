#pragma once
#include "view_obj.h"
#include <vector>

class barrier : public view_obj
{
public:
	//Math
	float* coefficients;
	int degree;	//degree of polynomial.
	float precision;
	float xmax;
	float xmin;

	//Sim
	int padding;

	barrier(float* coeff, int _degree, float _xmin, float _xmax, float prec, int padd, POINT origin);
	~barrier();


	bool isCollision(POINT test_pt);
	float getDerivative(float x);
	float f_of_x(float x);

	void draw(HDC hdc, HWND hwnd);
};

