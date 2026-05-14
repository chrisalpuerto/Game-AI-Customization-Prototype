#include "world.h"

#include <algorithm>
#include <cmath>

void WindBox::calculate_wind(float time, float world_width, float world_height) {
  int interval = 60;
  int half_interval = 30;
  int approx_interval_location = static_cast<int>(time * 10) % (interval * 10);
  float hshift = approx_interval_location - half_interval * 10;
  float prop = 1 - std::abs(hshift) / float(half_interval * 10);

  int x = static_cast<int>((world_width - wind_width) * prop);
  int y = (static_cast<int>(time / half_interval) % (static_cast<int>(world_height / wind_height) + 1)) *
          static_cast<int>(wind_height);

  top_left.x = x;
  top_left.y = y;
  SetRectPortable(&brc, top_left.x, top_left.y, top_left.x + static_cast<int>(wind_width),
                  top_left.y + static_cast<int>(wind_height));
  magnitude = max_magnitude * prop;
  dir = Vector(1, 0, 0);
}

bool WindBox::inBox(SimPoint pt) const {
  return PtInRectPortable(&brc, pt);
}

void RainBox::calculate_rain(float time, float world_width, float world_height) {
  int interval = 60;
  int half_interval = 30;
  int approx_interval_location = static_cast<int>(time * 10) % (interval * 10);
  float hshift = approx_interval_location - half_interval * 10;
  float prop = 1 - std::abs(hshift) / float(half_interval * 10);

  int x = (static_cast<int>(time / half_interval) % (static_cast<int>(world_width / rain_width) + 1)) *
          static_cast<int>(rain_width);
  int y = static_cast<int>((world_height - rain_height) * prop);
  top_left.x = x;
  top_left.y = y;
  SetRectPortable(&brc, top_left.x, top_left.y, top_left.x + static_cast<int>(rain_width),
                  top_left.y + static_cast<int>(rain_height));
  magnitude = max_magnitude * prop;
}

bool RainBox::inBox(SimPoint pt) const {
  return PtInRectPortable(&brc, pt);
}

SunBox::SunBox(float world_width, float world_height) {
  sun_width = 2.0f * world_width;
  sun_height = world_height;
}

void SunBox::calculate_sun(float time, float world_width) {
  int interval = 240;
  int dxdt = static_cast<int>((world_width + sun_width) / interval);
  top_left.x = static_cast<int>(world_width) - (static_cast<int>(time) % interval) * dxdt;
  top_left.y = 0;
  SetRectPortable(&brc, top_left.x, top_left.y, top_left.x + static_cast<int>(sun_width),
                  top_left.y + static_cast<int>(sun_height));
  magnitude = 80;
}

bool SunBox::inBox(SimPoint pt) const {
  return PtInRectPortable(&brc, pt);
}

World::World(int cols, int rows, int width, int height, float _delta_time)
    : num_columns(cols),
      num_rows(rows),
      world_width(width),
      world_height(height),
      sun(static_cast<float>(width), static_cast<float>(height)) {
  SetRectPortable(&brc, 0, 0, world_width, world_height);
  delta_time = _delta_time;

  int column_rem = world_width % num_columns;
  int row_rem = world_height % num_rows;
  int plot_width = (world_width - column_rem) / num_columns;
  int plot_height = (world_height - row_rem) / num_rows;
  land_width = plot_width;
  land_height = plot_height;

  for (int x = 0; x < num_columns; x++) {
    for (int y = 0; y < num_rows; y++) {
      int w = plot_width;
      int h = plot_height;
      SimPoint pt{x * w, y * h};
      if (x == num_columns - 1) w += column_rem;
      if (y == num_rows - 1) h += row_rem;

      land temp(pt, w, h);
      temp.nutrient_level = 100;
      temp.temperature = 50;
      temp.moisture_level = 50;
      temp.nut_regen = 5.5f;
      temp.mid_temp = (y % 2 == 0) ? 30.0f : 50.0f;
      temp.temp_radius = (x % 2 == 0) ? 30.0f : 10.0f;
      temp.calculate_fertility();
      land_plots.push_back(temp);
    }
  }
}

Vector World::get_wind(SimPoint pt) {
  if (wind.inBox(pt)) {
    Vector out = wind.dir;
    return out.MultiplyVectorByScalar(wind.magnitude);
  }
  return Vector(0, 0, 0);
}

void World::simulate_wind() {
  wind.calculate_wind(world_time, static_cast<float>(world_width), static_cast<float>(world_height));
}

void World::simulate_rain() {
  rain.calculate_rain(world_time, static_cast<float>(world_width), static_cast<float>(world_height));
}

void World::simulate_sun() {
  sun.calculate_sun(world_time, static_cast<float>(world_width));
}

void World::simulate_land() {
  for (size_t i = 0; i < land_plots.size(); i++) {
    land_plots[i].calculate_temperature(world_time);

    if (RectsOverlapPortable(rain.brc, land_plots[i].brc)) {
      land_plots[i].change_moisture(rain.magnitude);
    }

    if (land_plots[i].temperature > 70) {
      land_plots[i].change_moisture(-.005f * (land_plots[i].temperature / MAX_TEMPERATURE));
    }
    if (land_plots[i].temperature > 95) {
      land_plots[i].change_moisture(-1 * (land_plots[i].temperature / MAX_TEMPERATURE));
    }

    land_plots[i].change_nutrients(land_plots[i].nut_regen * delta_time);
    land_plots[i].set_sunlight(RectsOverlapPortable(sun.brc, land_plots[i].brc) ? sun.magnitude : 50);
    land_plots[i].calculate_fertility();
  }
}

void World::simulate_world(float step_time) {
  float final_time = world_time + step_time;

  while (world_time < final_time) {
    world_time += delta_time;
    simulate_wind();
    simulate_rain();
    simulate_sun();
    simulate_land();

    for (auto& object : objects) {
      if (!object || !object->alive) {
        continue;
      }

      object->accelerate(delta_time);
      object->move(delta_time);
      object->sync_view_loc();

      Barrier* bar = nullptr;
      if (checkBarrierCollisions(object->view_loc, &bar)) {
        object->Collision(50000, 0, 0);
        if (object->type == "cell") {
          static_cast<Cell*>(object.get())->feel_barrier();
        }
      }

      if (!PtInRectPortable(&brc, object->view_loc)) {
        object->Collision(300, 0, 0);
        if (object->type == "cell") {
          static_cast<Cell*>(object.get())->feel_barrier();
        }
        object->view_loc.x = std::clamp(object->view_loc.x, brc.left, brc.right);
        object->view_loc.y = std::clamp(object->view_loc.y, brc.top, brc.bottom);
        object->part_loc.tuple[0] = static_cast<float>(object->view_loc.x);
        object->part_loc.tuple[1] = static_cast<float>(object->view_loc.y);
      }

      if (object->type == "food" && wind.inBox(object->view_loc)) {
        Vector force = wind.dir.MultiplyVectorByScalar(wind.magnitude);
        object->applyForce(force);
      }
    }

    for (auto& object : objects) {
      if (object && object->alive && object->type == "cell") {
        static_cast<Cell*>(object.get())->simulate(*this, delta_time);
      } else if (object && object->alive) {
        object->tick(delta_time);
      }
    }

    objects.erase(std::remove_if(objects.begin(), objects.end(),
                                 [](const std::unique_ptr<Entity>& object) {
                                   return !object || !object->alive;
                                 }),
                  objects.end());
  }
}

Food* World::spawn_food(SimPoint pt, bool attach) {
  auto food = std::make_unique<Food>(pt);
  food->id = next_entity_id++;
  if (attach) {
    food->attach_to_point(myPoint(static_cast<float>(pt.x), static_cast<float>(pt.y), 0), 1000, 25);
  }
  Food* raw = food.get();
  objects.push_back(std::move(food));
  return raw;
}

Cell* World::spawn_cell(SimPoint pt, int id) {
  auto cell = std::make_unique<Cell>(pt, id);
  cell->id = id > 0 ? id : next_entity_id;
  next_entity_id = std::max(next_entity_id, cell->id + 1);
  cell->mass = 50;
  Cell* raw = cell.get();
  objects.push_back(std::move(cell));
  return raw;
}

Barrier* World::spawn_linear_barrier(SimPoint origin, float c0, float c1, float xmin, float xmax) {
  std::vector<float> coeff = {c0, c1};
  auto barrier = std::make_unique<Barrier>(coeff, 1, xmin, xmax, 5, 20, origin);
  barrier->id = next_entity_id++;
  barrier->mass = 500;
  Barrier* raw = barrier.get();
  barriers.push_back(raw);
  objects.push_back(std::move(barrier));
  return raw;
}

bool World::checkBarrierCollisions(SimPoint pt, Barrier** bar) {
  for (Barrier* current : barriers) {
    if (current && current->alive && current->isCollision(pt)) {
      *bar = current;
      return true;
    }
  }
  *bar = nullptr;
  return false;
}

land* World::getLand(SimPoint pt) {
  if (!PtInRectPortable(&brc, pt)) {
    return nullptr;
  }

  int i = pt.x / land_width;
  int j = pt.y / land_height;
  if (i == num_columns) i = num_columns - 1;
  if (j == num_rows) j = num_rows - 1;
  int idx = i * num_rows + j;
  return &land_plots[idx];
}

WorldSnapshot World::snapshot() const {
  WorldSnapshot snap;
  snap.time = world_time;
  snap.width = world_width;
  snap.height = world_height;

  for (const auto& object : objects) {
    if (!object) continue;

    EntitySnapshot item;
    item.id = object->id;
    item.type = object->type;
    item.x = object->part_loc.tuple[0];
    item.y = object->part_loc.tuple[1];
    item.vx = object->vel.tuple[0];
    item.vy = object->vel.tuple[1];
    item.alive = object->alive;

    if (object->type == "cell") snap.cells.push_back(item);
    else if (object->type == "food") snap.food.push_back(item);
    else if (object->type == "barrier") snap.barriers.push_back(item);
  }

  return snap;
}
