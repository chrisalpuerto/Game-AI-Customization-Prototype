#pragma once
#include "land.h"
#include "Vector.h"
#include "food.h"
#include "cell.h"
#include "plant.h"
#include "spore.h"
#include "barrier.h"

#include "myDynQueue.h"

#include <vector>


typedef struct wind
{
	RECT brc;			//bounding rectangle
	POINT top_left;		//Top left of current wind box.
	float wind_width;	//width of wind box
	float wind_height;	//height of wind box
	Vector dir;			//Direction of wind vectors
	float magnitude;	//Magnitude of force due to wind
	//int max_magnitude = 200; TEMPORARILY TURNING OFF WIND
	int max_magnitude = 0;

	wind()
	{
		wind_width = 200;
		wind_height = 200;
		magnitude = 0;
		brc.left = 0;
		brc.right = 0;
		brc.top = 0;
		brc.bottom = 0;
		top_left.x = 0;
		top_left.y = 0;
	}
	void calculate_wind(float time, float world_width, float world_height)
	{
		//============================================================================================================
		//First, calculate top_left of wind box.

		int interval = 60;
		int half_interval = 30;

		//Approximate to one decimal place. -> Range: 0 to 600
		int approx_interval_location = (int(time * 10) % (interval * 10));
		float hshift = approx_interval_location - half_interval * 10;
		float prop = 1 - abs(hshift) / float(half_interval * 10);

		//float corner_constant = (-abs((int(time * 10) % (interval * 10)) - half_interval*10) / (half_interval*10 + 10));
		

		int x = (world_width - wind_width )* prop;
		int y = (int(time/half_interval) % (int(world_height / wind_height) + 1)) * wind_height;

		//Set topleft of current wind box.
		top_left.x = x;
		top_left.y = y;
		//Set bonding rectangle of current wind box.
		SetRect(&brc, top_left.x, top_left.y, top_left.x + wind_width, top_left.y + wind_height);

		//============================================================================================================
		//Second, calculate magnitude of wind vectors.
		magnitude = max_magnitude * prop;
		
		
		//============================================================================================================
		//Third, calculate direction of wind vectors.

		//Here, interval is 1200 which means 120 seconds, to one decimal place.
		int rotate_interval = 10;
		//int angle = (360 / (rotate_interval*10)) * (int(time * 10) % (rotate_interval*10));

		Vector temp(1, 0, 0);	//points east
		//myMatrix mat;
		//mat.init_RotXY_3x3(angle);
		//mat.Rotate_XY_3x3(&temp, &temp);
		dir = temp;


	}
	bool inBox(POINT pt)
	{		
		return PtInRect(&brc, pt);
	}
};

typedef struct rain
{
	RECT brc;			//bounding rectangle
	POINT top_left;		//Top left of current wind box.
	float rain_width;	//width of rain box
	float rain_height;	//height of rain box
	float magnitude;	//Magnitude of moisture imparted by the rain
	int max_magnitude = 100;

	rain()
	{
		rain_width = 200;
		rain_height = 200;
		magnitude = 0;
		brc.left = 0;
		brc.right = 0;
		brc.top = 0;
		brc.bottom = 0;
		top_left.x = 0;
		top_left.y = 0;
	}
	void calculate_rain(float time, float world_width, float world_height)
	{
		//============================================================================================================
		//First, calculate top_left of rain box.

		int interval = 60;
		int half_interval = 30;

		//Approximate to one decimal place. -> Range: 0 to 600
		int approx_interval_location = (int(time * 10) % (interval * 10));
		float hshift = approx_interval_location - half_interval * 10;
		float prop = 1 - abs(hshift) / float(half_interval * 10);

		//float corner_constant = (-abs((int(time * 10) % (interval * 10)) - half_interval*10) / (half_interval*10 + 10));


		int x = (int(time / half_interval) % (int(world_width / rain_width) + 1)) * rain_width;
		int y = (world_height - rain_height) * prop;

		//Set topleft of current rain box.
		top_left.x = x;
		top_left.y = y;
		//Set bounding rectangle of current rain box.
		SetRect(&brc, top_left.x, top_left.y, top_left.x + rain_width, top_left.y + rain_height);

		//============================================================================================================
		//Second, calculate magnitude of rain vectors.
		magnitude = max_magnitude * prop;


	}
	void init_rain(int x, int y)
	{
		//Set topleft of current rain box.
		top_left.x = x;
		top_left.y = y;
		//Set bounding rectangle of current rain box.
		SetRect(&brc, top_left.x, top_left.y, top_left.x + rain_width, top_left.y + rain_height);
	}
	bool inBox(POINT pt)
	{
		return PtInRect(&brc, pt);
	}

};

typedef struct sun
{
	RECT brc;				//bounding rectangle.
	POINT top_left;			//top left of current sun box.
	float sun_width;		//width of sun box.
	float sun_height;		//height of sun box.
	float magnitude;		//0 to 100. could be used as an abstraction of cloudiness, for example, or other factors that would screen the sun. is the amount of sunlight in landplots that overlap with sun box.
	int max_magnitude = 100;
	int min_magnitude = 0;

	sun(float world_width, float world_height)
	{
		sun_width = 2.0 * world_width;
		sun_height = world_height;
		magnitude = 0;
		brc.left = 0;
		brc.right = 0;
		brc.top = 0;
		brc.bottom = 0;
		top_left.x = 0;
		top_left.y = 0;
	}

	void calculate_sun(float time, float world_width)
	{
		//A day shall be 240 seconds.
		int interval = 240;
		int dxdt = (world_width+sun_width) / interval;


		top_left.x = world_width - (int(time) % interval) * dxdt;
		top_left.y = 0;		//sun-box height always the same as world height.

		//Set bounding rectangle of current sun box.
		SetRect(&brc, top_left.x, top_left.y, top_left.x + sun_width, top_left.y + sun_height);

		//For now, nothing affects magnitude of sun, it will always bring sunlight at 80%.
		magnitude = 80;

		

	}

	bool inBox(POINT pt)
	{
		return PtInRect(&brc, pt);
	}

};

class world
{
public:
	int num_columns;
	int num_rows;
	int world_width;
	int world_height;
	int land_width;
	int land_height;

	RECT brc;

	//All times should be in seconds.
	float world_time;
	float delta_time;	

	std::vector<land> land_plots;

	POINT rain_box_top_left;
	POINT wind_box_top_left;

	world(int cols, int rows, int width, int height, float _delta_time);

	//===================================
	//Wind.
	wind WIND;

	//Get force vector due to wind at point, pt.
	Vector get_wind(POINT pt);
	void simulate_wind();


	//=====================================
	//Rain
	rain RAIN;
	vector<rain> rains;
	bool find_spore = false;
	POINT spore_loc;
	void simulate_rain();

	//=====================================
	//Sun
	sun* SUN;
	void simulate_sun();


	//====================================
	//Land.
	void simulate_land();



	//====================================
	//World
	//step_time = how much time should be simulated since last call to simulate_world
	void simulate_world(float step_time);



	//Entities
	DynQueue<view_obj*> objects;
	DynQueue<barrier*> barriers;

	food* spawn_food(POINT pt, bool attach);
	void spawn_cell(POINT pt, int id);
	void spawn_plant(POINT pt, Vector dir, int seg_length);
	void spawn_spore(POINT pt, Vector dir, float initial_speed);

	void spawn_linear_barrier(POINT origin, float c0, float c1, float xmin, float xmax);
	void spawn_parabolic_barrier(POINT origin, float c0, float c1, float c2, float xmin, float xmax);
	void spawn_cubic_barrier(POINT origin, float c0, float c1, float c2, float c3, float xmin, float xmax);

	bool checkBarrierCollisions(POINT pt, barrier** bar);



	bool isOverlapping(RECT r1, RECT r2);
	land* getLand(POINT pt);

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

