#include "land.h"

#include <algorithm>
#include <cmath>

land::land(SimPoint _top_left, int width, int height) {
  top_left = _top_left;
  land_width = width;
  land_height = height;
  SetRectPortable(&brc, top_left.x, top_left.y, top_left.x + land_width, top_left.y + land_height);
}

land::~land() {}

void land::set_sunlight(float value) {
  sunlight = std::clamp(value, 0.0f, 100.0f);
}

void land::change_nutrients(float delta) {
  nutrient_level = std::clamp(nutrient_level + delta, 0.0f, static_cast<float>(MAX_NUTRIENTS));
}

void land::change_moisture(float delta) {
  moisture_level = std::clamp(moisture_level + delta, 0.0f, static_cast<float>(MAX_MOISTURE));
}

void land::calculate_temperature(float time) {
  temperature = mid_temp + temp_radius * std::sin(time / 20.0f);
  temperature = std::clamp(temperature, 0.0f, static_cast<float>(MAX_TEMPERATURE));
}

void land::calculate_fertility() {
  float nutrient_score = nutrient_level / MAX_NUTRIENTS;
  float moisture_score = moisture_level / MAX_MOISTURE;
  float sunlight_score = sunlight / 100.0f;
  float temperature_score = 1.0f - std::abs(50.0f - temperature) / 50.0f;
  fertility = std::clamp((nutrient_score + moisture_score + sunlight_score + temperature_score) / 4.0f, 0.0f, 1.0f);
}

bool land::point_in_land(SimPoint pt) {
  return PtInRectPortable(&brc, pt);
}

