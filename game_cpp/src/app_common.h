#pragma once
#include <Windows.h>
#include "myPoint.h"

//don't include this file.


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