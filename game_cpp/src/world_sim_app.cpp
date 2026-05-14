


#include "world_sim_app.h"
world_sim_app::world_sim_app(HWND hwnd, HDC hdc)
{
	fullscreen = true;		//not currently necessary here. global in main used instead.
	m_hwnd = hwnd;
	m_hdc = hdc;
	selected_module = 0;

	HFONT hFont = CreateFont(
		56,                // Height of font
		0,								// Width of font
		0,								// Angle of escapement
		0,								// Orientation angle
		FW_NORMAL,						// Font weight
		FALSE,							// Italic attribute option
		FALSE,							// Underline attribute option
		FALSE,							// Strikeout attribute option
		ANSI_CHARSET,					// Character set identifier
		OUT_DEFAULT_PRECIS,				// Output precision
		CLIP_DEFAULT_PRECIS,			// Clipping precision
		DEFAULT_QUALITY,				// Output quality
		DEFAULT_PITCH | FF_SWISS,		// Pitch and family
		TEXT("Arial"));					 // Font face name

	m_header_font = hFont;
}
world_sim_app::~world_sim_app()
{
	//Clean up allocated memory.
	delete header_text;
	objects.clear();
}
void world_sim_app::setDC(HDC hdc)
{
	m_hdc = hdc;
}
void world_sim_app::setHWND(HWND hwnd)
{
	m_hwnd = hwnd;
}
void world_sim_app::init()
{
	running_time = 0;
	bgColor = COLORREF(0x993300);
	RECT clirect;
	GetClientRect(m_hwnd, &clirect);
	int clirect_height = clirect.bottom - clirect.top;
	int clirect_width = clirect.right - clirect.left;



	header.left = clirect.left;
	header.right = clirect.right;
	header.top = clirect.top;
	header.bottom = clirect.top + 50;

	header_text = new WCHAR[13];
	header_text = { L"Basic 2D World\0" };

	init_module(selected_module);


	/*
	POINT fd1_pt;
	fd1_pt.x = 100;
	fd1_pt.y = 100;
	food* fd1 = new food(fd1_pt);
	fd1->mass = 20;
	view_obj* o_temp = fd1;
	objects.enqueue(reinterpret_cast<view_obj*>(fd1));

	
	POINT c1_pt;
	c1_pt.x = 200;
	c1_pt.y = 200;
	cell* c1 = new cell(c1_pt);
	c1->mass = 50;
	objects.enqueue(reinterpret_cast<view_obj*>(c1));
	*/

	init_world(1);
	//WORLD->simulate_world(700);
}


void world_sim_app::draw_wind(HDC hdc)
{
	RECT windbox = WORLD->WIND.brc;

	//The following works because every wind_vec is the same within the windbox.
	Vector wind_vec = WORLD->WIND.dir.MultiplyVectorByScalar(WORLD->WIND.magnitude);

	for (int i = windbox.left; i < windbox.right; i += 10)
	{
		for (int j = windbox.top; j < windbox.bottom; j += 10)
		{
			myPoint pt1(i, j, 0);
			myPoint pt2 = pt1.AddVectorToPoint(&wind_vec);

			POINT wpt1 = myPoint_to_winPoint(pt1);
			POINT wpt2 = myPoint_to_winPoint(pt2);

			HPEN greyPen = CreatePen(PS_SOLID, 2, COLORREF(0xDDDDDD));
			SelectObject(hdc, greyPen);
			MoveToEx(hdc, wpt1.x, wpt1.y, NULL);
			LineTo(hdc, wpt2.x, wpt2.y);
			DeleteObject(greyPen);
		}
	}
}

void world_sim_app::draw_land(HDC hdc)
{
	int max_red = 0x75;
	int max_green = 0x41;
	int max_blue = 0x01;

	int min_red = 0xeb;
	int min_green = 0x74;
	int min_blue = 0x0a;

	//Slopes w.r.t fertility
	int dr = max_red - min_red;
	int dg = max_green - min_green;
	int db = max_blue - min_blue;

	for (int i = 0; i < WORLD->land_plots.size(); i++)
	{	
		RECT brc = WORLD->land_plots[i].brc;
		float fertility = WORLD->land_plots[i].fertility;
		float sunlight = WORLD->land_plots[i].sunlight;
		
		//Base color
		int red = min_red + dr * fertility;
		int green = min_green + dg * fertility;
		int blue = min_blue + db * fertility;

		//Sunlight modifier
		//sunlight = 0 -> land is black; sunlight = 100 -> land is its base color
		//Therefore, slopes are equal to the color components themselves.
		float sun_percent = sunlight / 100;
		red = (.0 * red) + red * sun_percent;
		green = (.0 * green) + green * sun_percent;
		blue = (.0 * blue) + blue * sun_percent;


		COLORREF color = (blue << 16) + (green << 8) + (red);
		HBRUSH brush = CreateSolidBrush(color);
		FillRect(hdc, &brc, brush);
		DeleteObject(brush);
	}

}

void world_sim_app::draw_rain(HDC hdc)
{
	RECT rainbox = WORLD->RAIN.brc;

	int max = WORLD->RAIN.rain_width / 20;
	int radius = 3;

	HPEN bluePen = CreatePen(PS_SOLID, 2, COLORREF(0xFF0000));
	HBRUSH blueBrush = CreateSolidBrush(COLORREF(0xFF0000));
	SelectObject(hdc, bluePen);
	SelectObject(hdc, blueBrush);

	for (int i = rainbox.left; i < rainbox.right; i += 20)
	{
		for (int j = rainbox.top; j < rainbox.bottom; j += 20)
		{
			int idx = j * max + i;
			if (int(idx * running_time) % 100 < 50)
			{
				RECT brc_droplet;
				brc_droplet.left = i - radius;
				brc_droplet.right = i + radius;
				brc_droplet.top = j - radius;
				brc_droplet.bottom = j + radius;

				Ellipse(hdc, brc_droplet.left, brc_droplet.top, brc_droplet.right, brc_droplet.bottom);

			}
		}
	}

	for (int r = 0; r < WORLD->rains.size(); r++)
	{
		rainbox = WORLD->rains[r].brc;
		max = WORLD->rains[r].rain_width / 20;
		for (int i = rainbox.left; i < rainbox.right; i += 20)
		{
			for (int j = rainbox.top; j < rainbox.bottom; j += 20)
			{
				int idx = j * max + i;
				if (int(idx * running_time) % 100 < 50)
				{
					RECT brc_droplet;
					brc_droplet.left = i - radius;
					brc_droplet.right = i + radius;
					brc_droplet.top = j - radius;
					brc_droplet.bottom = j + radius;

					Ellipse(hdc, brc_droplet.left, brc_droplet.top, brc_droplet.right, brc_droplet.bottom);

				}
			}
		}
	}

	DeleteObject(bluePen);
	DeleteObject(blueBrush);
}

void world_sim_app::draw_entities(HDC hdc)
{
	DynQueue<view_obj*>::Node* curObj = nullptr;
	WORLD->objects.traverse(&curObj);
	while (curObj)
	{
		if (curObj->data)
		{
			if (curObj->data->alive)
			{
				curObj->data->draw(hdc, m_hwnd);
			}
			WORLD->objects.traverse(&curObj);
		}
	}
}

void world_sim_app::Draw()
{
	//Set up back-buffer for device context.
	RECT clirect;
	GetClientRect(m_hwnd, &clirect);
	HDC memdc = CreateCompatibleDC(m_hdc);
	HBITMAP hmem_map = CreateCompatibleBitmap(m_hdc, clirect.right - clirect.left, clirect.bottom - clirect.top);
	HBITMAP oldbm = (HBITMAP)SelectObject(memdc, hmem_map);

	button_events();
	if (selected_module == 0)
	{
		//Running _______?
		//main_menu->Draw(memdc, m_hwnd);
		HBRUSH bgBrush = CreateSolidBrush(bgColor);
		FillRect(memdc, &clirect, bgBrush);
		DeleteObject(bgBrush);

		//Note, simulation calls should perhaps be moved outside draw functions such as this to maintain better architecture.
		//But it's ok for this simple test.
		WORLD->simulate_world(WORLD->delta_time);



		draw_land(memdc);
		draw_wind(memdc);
		draw_rain(memdc);
		draw_entities(memdc);


#if 0
		DynQueue<view_obj*>::Node* curObj = nullptr;
		objects.traverse(&curObj);
		while (curObj)
		{
			if (curObj->data->alive == false)
			{
				DynQueue<view_obj*>::Node* temp = curObj;
				objects.traverse(&curObj);
				objects.deleteNode(temp);				
			}
			else
			{
				curObj->data->draw(memdc, m_hwnd);
				curObj->data->accelerate(.33);
				curObj->data->view_move(.33);

				if (curObj->data->type == "cell")
				{
					int num_objects = objects.getCount();
					cell* pCell = reinterpret_cast<cell*>(curObj->data);
					pCell->see(&objects, num_objects);
					if (pCell->view_loc.x >= clirect.right || pCell->view_loc.x <= clirect.left || pCell->view_loc.y >= clirect.bottom || pCell->view_loc.y <= clirect.top)
					{
						//pCell->force_stop();
						pCell->Collision(100, 0, 0);
						pCell->feel_barrier();
					}
					pCell->think();
				}





				objects.traverse(&curObj);
			}

			
		}
#endif


		
		
		
	}


	//Blt buffer into the device context
	BitBlt(m_hdc, clirect.left, clirect.top, clirect.right - clirect.left, clirect.bottom - clirect.top, memdc, clirect.left, clirect.top, SRCCOPY);

	//Delete resources
	SelectObject(memdc, oldbm);
	DeleteObject(hmem_map);
	DeleteObject(memdc);

}

void world_sim_app::toggleFullscreen()
{
	if (fullscreen)
	{
		SetWindowLong(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
	}
	else
	{
		SetWindowLong(m_hwnd, GWL_STYLE, 0);
	}
	fullscreen != fullscreen;
}

void world_sim_app::process_left_click(POINT cursor_location)
{
	WORLD->spawn_food(cursor_location, false);
	//barrier* bar = nullptr;
	//WORLD->checkBarrierCollisions(cursor_location, &bar);
}
void world_sim_app::process_left_mouse_up(WPARAM wparam, LPARAM lparam)
{

}
void world_sim_app::process_drag(POINT cursor)
{

}
void world_sim_app::process_mouse_move(WPARAM wparam, LPARAM lparam)
{

}
void world_sim_app::endDrag()
{

}
void world_sim_app::mainResize(RECT clirect)
{

}
void world_sim_app::calculateLayout(RECT clirect)
{

}
void world_sim_app::process_scroll(POINT pt, int zdelta, int mult)
{

}
void world_sim_app::button_events()
{

}




void world_sim_app::key_w()
{
	DynQueue<view_obj*>::Node* curObj;
	objects.jump_to_index(1, curObj);

	Vector f(0, -1000, 0);
	if (curObj)
		curObj->data->applyForce(f);
}
void world_sim_app::key_a()
{
	DynQueue<view_obj*>::Node* curObj;
	objects.jump_to_index(1, curObj);
	Vector f(-1000, 0, 0);
	if (curObj)
		curObj->data->applyForce(f);
}
void world_sim_app::key_s()
{
	DynQueue<view_obj*>::Node* curObj;
	objects.jump_to_index(1, curObj);
	Vector f(0, 1000, 0);
	if (curObj)
		curObj->data->applyForce(f);
}
void world_sim_app::key_d()
{
	DynQueue<view_obj*>::Node* curObj;
	objects.jump_to_index(1, curObj);
	Vector f(1000, 0, 0);
	if (curObj)
		curObj->data->applyForce(f);
}

void world_sim_app::key_space()
{
	DynQueue<view_obj*>::Node* curObj;
	objects.jump_to_index(1, curObj);
	if (curObj)
		curObj->data->force_stop();
}








void world_sim_app::spawn_food(POINT pt)
{
	food* fd1 = new food(pt);
	fd1->mass = 20;
	objects.enqueue(reinterpret_cast<view_obj*>(fd1));
}

void world_sim_app::spawn_cell(POINT pt, int id)
{
	cell* c1 = new cell(pt, id);
	c1->mass = 50;
	objects.enqueue(reinterpret_cast<view_obj*>(c1));
}
void world_sim_app::init_world(int seed)
{
	//seed will be used to select a world configuration.

	int num_world_configs = 2;
	int idx = abs(seed) % num_world_configs;

	if (idx == 0)
	{
		RECT clirect;
		GetClientRect(m_hwnd, &clirect);
		int cliWidth = clirect.right - clirect.left;
		int cliHeight = clirect.bottom - clirect.top;

		int fwidth = 100;
		int fheight = 100;

		int num_columns = cliWidth / fwidth;
		int num_rows = cliHeight / fheight;

		WORLD = new world(num_columns, num_rows, cliWidth, cliHeight, .333);

		//Spawn food pattern.
		for (int i = 0; i < num_columns * num_rows; i++)
		{			
			POINT pt;
			pt.x = ((i % num_columns) * fwidth) + fwidth/2;
			pt.y = ((i / num_columns) * fheight) + fheight/2;
			WORLD->spawn_food(pt, false);
		}

		//Spawn cells pattern.
		int num_cells = 2;
		for (int i = 0; i < num_cells; i++)
		{
			int arg = (i + 1) * 10;
			POINT pt;
			pt.x = (cliWidth / 2) * cos(arg) + (cliWidth / 2);
			pt.y = (cliHeight / 2) * sin(arg) + (cliHeight / 2);
			WORLD->spawn_cell(pt, i);
		}

	}
	if (idx == 1)
	{
		RECT clirect;
		GetClientRect(m_hwnd, &clirect);
		int cliWidth = clirect.right - clirect.left;
		int cliHeight = clirect.bottom - clirect.top;

		WORLD = new world(128, 128, cliWidth, cliHeight, .333);

		//Spawn cells pattern.
		int num_cells = 5;
		for (int i = 0; i < num_cells; i++)
		{
			int arg = (i + 1) * 10;
			POINT pt;
			pt.x = (cliWidth / 2) * cos(arg) + (cliWidth / 2);
			pt.y = (cliHeight / 2) * sin(arg) + (cliHeight / 2);
			WORLD->spawn_cell(pt, i+1);
		}

		//Spawn plants.
		/*
		POINT pt;
		pt.x = 100;
		pt.y = 100;
		plant* p1 = new plant(pt, Vector(1,0,0), 5, WORLD);
		p1->mass = 100;
		WORLD->objects.enqueue(reinterpret_cast<view_obj*>(p1));


		
		pt.x = 1000;
		pt.y = 500;
		plant* p2 = new plant(pt, Vector(-1, -1, 0), 1, WORLD);
		p2->mass = 100;
		WORLD->objects.enqueue(reinterpret_cast<view_obj*>(p2));

		pt.x = 100;
		pt.y = 700;
		plant* p3 = new plant(pt, Vector(1, 1, 0), 10, WORLD);
		p3->mass = 100;
		WORLD->objects.enqueue(reinterpret_cast<view_obj*>(p3));
		*/

		POINT pt;
		pt.x = 300;
		pt.y = 300;
		spore* s1 = new spore(pt, WORLD);
		s1->mass = 5;
		WORLD->objects.enqueue(reinterpret_cast<view_obj*>(s1));

		/*
		pt.x = 1200;
		pt.y = 600;
		spore* s2 = new spore(pt, WORLD);
		s2->mass = 5;
		WORLD->objects.enqueue(reinterpret_cast<view_obj*>(s2));
		*/



		for (int i = 0; i < WORLD->land_plots.size(); i++)
		{
			if ((WORLD->land_plots[i].top_left.x > WORLD->world_width / 2))
			{
				WORLD->land_plots[i].nut_regen = 1;
				WORLD->land_plots[i].mid_temp = 95;
				WORLD->land_plots[i].temp_radius = 5;
				if ((WORLD->land_plots[i].top_left.y > WORLD->world_height / 2))
				{
					WORLD->land_plots[i].nut_regen = 8;
					WORLD->land_plots[i].mid_temp = 70;
					WORLD->land_plots[i].temp_radius = 5;
				}
			}
		}

		rain nRain;
		nRain.magnitude = 50;
		nRain.rain_width = WORLD->world_width;
		nRain.rain_height = WORLD->world_height;
		nRain.init_rain(0, 0);
		WORLD->rains.push_back(nRain);		
		

		//Add a test barrier
		POINT b_org;
		b_org.x = 900;
		b_org.y = 400;
		//WORLD->spawn_linear_barrier(b_org, 0, -1, -200, 200);
		//WORLD->spawn_parabolic_barrier(b_org, 0, 0, -.001, -600, 600);
		WORLD->spawn_cubic_barrier(b_org, 0, 0, .001, -.0001, -600, 600);

		


	}


}



void world_sim_app::init_module(int curmod)
{

}

void world_sim_app::CloseModule()
{

}