#include "food.h"

food::food(POINT loc)
{
	radius = 10;
	view_loc = loc;
	part_loc = winPoint_to_myPoint(loc);
	type = "food";

	max_speed = 30;

	alive = true;
	energy = 1000;
}
food::~food()
{

}
void food::draw(HDC hdc, HWND hwnd)
{
	HPEN blackPen = CreatePen(PS_SOLID, 2, COLORREF(0x000000));
	HBRUSH grnBrush = CreateSolidBrush(COLORREF(0x00FF00));
	RECT brc;
	brc.left = view_loc.x - radius;
	brc.right = view_loc.x + radius;
	brc.top = view_loc.y - radius;
	brc.bottom = view_loc.y + radius;
	SelectObject(hdc, grnBrush);
	SelectObject(hdc, blackPen);
	Ellipse(hdc, brc.left, brc.top, brc.right, brc.bottom);
	for (int i = 0; i < attachements.size(); i++)
	{
		POINT atpt = myPoint_to_winPoint(attachements[i].pt);
		MoveToEx(hdc, view_loc.x, view_loc.y, NULL);
		LineTo(hdc, atpt.x, atpt.y);
	}


	DeleteObject(blackPen);
	DeleteObject(grnBrush);




}