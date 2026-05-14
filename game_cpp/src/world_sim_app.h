

#pragma once
#include <Windows.h>
#include "world.h"
#include "food.h"
#include "cell.h"
#include "myDynQueue.h"
class world_sim_app
{
private:
	RECT header;
	HWND m_hwnd;
	HDC m_hdc;
	COLORREF bgColor;
	const WCHAR* header_text;
	HFONT m_header_font;
	bool fullscreen;
public:
	world_sim_app(HWND hwnd, HDC hdc);
	~world_sim_app();
	void setDC(HDC hdc);
	void setHWND(HWND hwnd);
	void init();
	void Draw();

	void toggleFullscreen();

	void process_left_click(POINT cursor_location);
	void process_left_mouse_up(WPARAM wparam, LPARAM lparam);
	void process_drag(POINT cursor);
	void process_mouse_move(WPARAM wparam, LPARAM lparam);
	void endDrag();
	void mainResize(RECT clirect);
	void calculateLayout(RECT clirect);
	void process_scroll(POINT pt, int zdelta, int mult);

	void key_w();
	void key_a();
	void key_s();
	void key_d();
	void key_space();

	void button_events();


	void draw_wind(HDC hdc);
	void draw_land(HDC hdc);
	void draw_rain(HDC hdc);
	void draw_entities(HDC hdc);


	float running_time;


	int selected_module;
	void init_module(int curmod);
	int closing_module; //idx of the module that was running and must be closed.
	void CloseModule();
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//modules
	//to-do, if necessary.




	//Model
	world* WORLD;
	DynQueue<view_obj*> objects;

	void spawn_food(POINT pt);
	void spawn_cell(POINT pt, int id);
	void init_world(int seed);





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

