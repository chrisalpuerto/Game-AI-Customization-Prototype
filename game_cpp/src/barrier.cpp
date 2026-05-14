#include "barrier.h"


barrier::barrier(float* coeff, int _degree, float _xmin, float _xmax, float prec, int padd, POINT origin)
{
	coefficients = coeff;
	degree = _degree;
	padding = padd;
	precision = prec;
	xmin = _xmin;
	xmax = _xmax;
	view_loc = origin;
	part_loc = winPoint_to_myPoint(origin);
	alive = true;
}
barrier::~barrier()
{
	delete coefficients;
}



bool barrier::isCollision(POINT test_pt)
{
	//Prelimnary: convert test_pt to non-translated coordinates.
	POINT test_pt_prime;
	test_pt_prime.x = test_pt.x - view_loc.x;
	test_pt_prime.y = test_pt.y - view_loc.y;



	//First, find the point, curve_pt, which defines the shortest secant line (perp. to tan.) of the curve that passes through the test point.
	POINT curve_pt;
	bool found = false;

	float error_margin = 200;
	float prec = .1;
	for (float x = xmin; (x < xmax); x += prec)
	{
		//Derivative of the polynomial
		float dydx = getDerivative(x);
		float y = f_of_x(x);

		float diff = dydx + (x - test_pt_prime.x) / (y - test_pt_prime.y);
		if (abs(diff) < .1)
		{
			//Close enough to the perpendicular line we seek.

			curve_pt.x = x;
			curve_pt.y = y;
			found = true;
			break;
		}
	}


	//Next, check length of line from test_pt to curve_pt.
	if (found)
	{
		float dist = sqrt(pow((curve_pt.x - test_pt_prime.x), 2) + pow((curve_pt.y - test_pt_prime.y), 2));

		if (dist <= padding)
		{
			return true;
		}
	}
	return false;

}

float barrier::getDerivative(float x)
{
	float* curr = coefficients;
	float dydx = 0;
	for (int i = 0; i <= degree; i++)
	{
		dydx += *curr * i * pow(x, i - 1);
		curr++;
	}
	return dydx;
}

float barrier::f_of_x(float x)
{
	//Maybe in future, update to return function value in pointer variable passed as argument and set return value of this function to a bool indicating whether or not the function is defined at the parameter given.

	float* curr = coefficients;
	float y = 0;
	int i = 0;
	while (i <= degree)
	{
		y += *curr * pow(x, i);
		i++;
		curr++;
	}
	return y;
}


void barrier::draw(HDC hdc, HWND hwnd)
{
	std::vector<POINT> pts;

	//note: using a scaling factor might be advisable in future.
	
	
	for (float x = xmin; x < xmax; x += precision)
	{
		float* curr = coefficients;
		float y = 0;
		int i = 0;
		while (i<= degree)
		{
			y += *curr * pow(x, i);
			i++;
			curr++;
		}
		POINT pt;
		pt.x = x + view_loc.x;
		pt.y = y + view_loc.y;
		pts.push_back(pt);
	}
	HPEN pen = CreatePen(PS_SOLID, 3, COLORREF(0x000000));
	SelectObject(hdc, pen);
	MoveToEx(hdc, pts[0].x, pts[0].y, NULL);
	for (int i = 1; i < pts.size(); i++)
	{
		LineTo(hdc, pts[i].x, pts[i].y);
	}
	DeleteObject(pen);
}