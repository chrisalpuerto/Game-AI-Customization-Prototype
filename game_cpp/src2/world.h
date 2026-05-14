#pragma once

#include "Vector.h"
#include "barrier.h"
#include "cell.h"
#include "food.h"
#include "land.h"
#include "platform_types.h"

#include <memory>
#include <string>
#include <vector>

struct WindBox {
  SimRect brc;
  SimPoint top_left;
  float wind_width = 200;
  float wind_height = 200;
  Vector dir;
  float magnitude = 0;
  int max_magnitude = 0;

  WindBox() : dir(1, 0, 0) {}
  void calculate_wind(float time, float world_width, float world_height);
  bool inBox(SimPoint pt) const;
};

struct RainBox {
  SimRect brc;
  SimPoint top_left;
  float rain_width = 200;
  float rain_height = 200;
  float magnitude = 0;
  int max_magnitude = 100;

  void calculate_rain(float time, float world_width, float world_height);
  void init_rain(int x, int y);
  bool inBox(SimPoint pt) const;
};

struct SunBox {
  SimRect brc;
  SimPoint top_left;
  float sun_width = 0;
  float sun_height = 0;
  float magnitude = 0;

  SunBox(float world_width, float world_height);
  void calculate_sun(float time, float world_width);
  bool inBox(SimPoint pt) const;
};

struct EntitySnapshot {
  int id = 0;
  std::string type;
  float x = 0;
  float y = 0;
  float vx = 0;
  float vy = 0;
  bool alive = true;
};

struct WorldSnapshot {
  float time = 0;
  int width = 0;
  int height = 0;
  std::vector<EntitySnapshot> cells;
  std::vector<EntitySnapshot> food;
  std::vector<EntitySnapshot> barriers;
};

class World {
public:
  int num_columns;
  int num_rows;
  int world_width;
  int world_height;
  int land_width;
  int land_height;
  SimRect brc;
  float world_time = 0;
  float delta_time = 0;
  std::vector<land> land_plots;
  WindBox wind;
  RainBox rain;
  std::vector<RainBox> rains;
  SunBox sun;
  std::vector<std::unique_ptr<Entity>> objects;
  std::vector<Barrier*> barriers;
  int next_entity_id = 1;

  World(int cols, int rows, int width, int height, float _delta_time);

  Vector get_wind(SimPoint pt);
  void simulate_wind();
  void simulate_rain();
  void simulate_sun();
  void simulate_land();
  void simulate_world(float step_time);

  Food* spawn_food(SimPoint pt, bool attach);
  Cell* spawn_cell(SimPoint pt, int id);
  Barrier* spawn_linear_barrier(SimPoint origin, float c0, float c1, float xmin, float xmax);
  bool checkBarrierCollisions(SimPoint pt, Barrier** bar);
  land* getLand(SimPoint pt);
  WorldSnapshot snapshot() const;
};
