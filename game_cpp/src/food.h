#pragma once
#include "view_obj.h"
#include "windows.h"

class food : public view_obj
{
public:
	float energy;
	int radius;
	//float energy;
	food(POINT loc);
	~food();
	void draw(HDC hdc, HWND hwnd);

};

