#pragma once
#include "windows.h"
#include "particle.h"
#include <string>
class view_obj : public particle
{
public:
	std::string type;		//object type: cell, food, etc.
	bool alive;				//alive or in tact/instantiated. Basically, in the simulation or not (anymore, presumably).
	POINT view_loc;
	virtual void draw(HDC hdc, HWND hwnd) = 0;
	void view_move(float dt);

	POINT myPoint_to_winPoint(myPoint in_pt)
	{
		POINT pt;
		pt.x = in_pt.tuple[0];
		pt.y = in_pt.tuple[1];
		return pt;
	}

	myPoint winPoint_to_myPoint(POINT in_pt)
	{
		myPoint pt;
		pt.tuple[0] = in_pt.x;
		pt.tuple[1] = in_pt.y;
		return pt;
	}
};

