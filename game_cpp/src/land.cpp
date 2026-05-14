#include "land.h"

land::land(POINT _top_left, int width, int height)
{
	nutrient_level = 0;
	temperature = 0;
	moisture_level = 0;
	fertility = 0;
	sunlight = 0;

	top_left = _top_left;
	land_width = width;
	land_height = height;

	SetRect(&brc, top_left.x, top_left.y, top_left.x + land_width, top_left.y + land_height);

}

land::~land()
{

}

void land::set_sunlight(float value)
{
	sunlight = value;
}

void land::change_nutrients(float delta)
{
	//delta > 0 -> nutrients increase
	//delta < 0 -> nutrients decrease
	nutrient_level += delta;
	if (nutrient_level < 0)
	{
		nutrient_level = 0;
	}
	if (nutrient_level > MAX_NUTRIENTS)
	{
		nutrient_level = MAX_NUTRIENTS;
	}
}

void land::change_moisture(float delta)
{
	//moisture is in percentage.
	//so min: 0 and max: 100.

	moisture_level += delta;
	if (moisture_level < 0)
	{
		moisture_level = 0;
	}
	if (moisture_level > MAX_MOISTURE)
	{
		moisture_level = MAX_MOISTURE;
	}
}

void land::calculate_temperature(float time)
{
	//time must be the time elapsed since the beginning of the simulation, NOT a time difference.

	//Play with c to adjust the speed of temperature oscillation.
	float c = 1;
	temperature = temp_radius*sin(time*c) + mid_temp;
	if (temperature < 0)
	{
		temperature = 0;
	}
	if (temperature > MAX_TEMPERATURE)
	{
		temperature = MAX_TEMPERATURE;
	}
}

void land::calculate_fertility()
{
	//fertility is a summary variable that amounts to an average of moisture, temperature, and nutrient level.
	//divide by 2 because, for now, only taking two factors into account.
	fertility = (nutrient_level / MAX_NUTRIENTS + moisture_level / MAX_MOISTURE) / 2;
	//fertility += temperature / 100;
	
	if (fertility < 0)
	{
		fertility = 0;
	}
	if (fertility > 1)
	{
		fertility = 1;
	}

}

bool land::point_in_land(POINT pt)
{
	return PtInRect(&brc, pt);
}