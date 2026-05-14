#pragma once

#include "platform_types.h"

#define MAX_NUTRIENTS 500
#define MAX_MOISTURE 100
#define MAX_TEMPERATURE 100

class land {
public:
  SimRect brc;
  SimPoint top_left;
  int land_width = 0;
  int land_height = 0;
  float sunlight = 0;
  float nutrient_level = 0;
  float temperature = 0;
  float moisture_level = 0;
  float fertility = 0;
  float nut_regen = 0;
  float mid_temp = 0;
  float temp_radius = 0;

  land(SimPoint _top_left, int width, int height);
  ~land();

  void set_sunlight(float value);
  void change_nutrients(float delta);
  void change_moisture(float delta);
  void calculate_temperature(float time);
  void calculate_fertility();
  bool point_in_land(SimPoint pt);
};

