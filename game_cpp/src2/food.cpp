#include "food.h"

Food::Food(SimPoint loc) {
  type = "food";
  view_loc = loc;
  part_loc = myPoint(static_cast<float>(loc.x), static_cast<float>(loc.y), 0);
  radius = 12;
  mass = 20;
  max_speed = 15;
}

Food::~Food() {}

void Food::tick(float) {
  sync_view_loc();
}

