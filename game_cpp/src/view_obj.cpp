#include "view_obj.h"

void view_obj::view_move(float dt)
{
	move(dt);
	view_loc = myPoint_to_winPoint(part_loc);
}