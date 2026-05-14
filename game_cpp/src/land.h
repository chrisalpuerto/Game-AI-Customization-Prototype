#pragma once
#include <Windows.h>
#include <cmath>

#define MAX_NUTRIENTS 500
#define MAX_MOISTURE 100
#define MAX_TEMPERATURE 100

class land
{
public:
	//Representation/Geometrical details
	RECT brc;
	POINT top_left;
	int land_width;
	int land_height;

	//Geological details
	float sunlight;
	float nutrient_level;
	float temperature;
	float moisture_level;
	float fertility;

	float nut_regen;	//Rate w.r.t. time (in seconds) of nutrition regeneration in this land plot.
	float mid_temp;     //Center of temperature range
	float temp_radius;   //such that min_temp = mid_temp - temp_radius and max_temp = mid_temp + temp_radius



	land(POINT _top_left, int width, int height);
	~land();

	void set_sunlight(float value);
	void change_nutrients(float delta);
	void change_moisture(float delta);
	void calculate_temperature(float time);
	void calculate_fertility();

	bool point_in_land(POINT pt);
};

