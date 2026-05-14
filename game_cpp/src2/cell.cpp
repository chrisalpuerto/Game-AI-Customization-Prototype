#include "cell.h"

#include "food.h"
#include "myMatrix.h"
#include "world.h"

#include <algorithm>
#include <cmath>

Cell::Cell(SimPoint loc, int _id) {
  max_speed = 30;
  radius = 20;
  sight_height = 500;
  sight_angle = 50;
  sight_dir = Vector(1, 0, 0);
  view_loc = loc;
  part_loc = myPoint(static_cast<float>(loc.x), static_cast<float>(loc.y), 0);
  type = "cell";
  alive = true;
  cell_id = _id;
  goal_point = part_loc;
}

Cell::~Cell() {}

void Cell::tick(float) {
  sync_view_loc();
}

void Cell::energy_depletion(float delta) {
  energy += delta;
  if (energy <= 0) {
    alive = false;
  }

  float prop = energy / 15000.0f;
  if (prop <= 0.7f) {
    hungry = true;
    bored = false;
  }
}

void Cell::rotate_eyes(float angle) {
  myMatrix mat;
  mat.init_RotXY_3x3(angle);
  mat.Rotate_XY_3x3(&sight_dir, &sight_dir);
}

void Cell::move_forward(float force) {
  energy_depletion(-1);
  Vector f = sight_dir.MultiplyVectorByScalar(force);
  applyForce(f);
}

void Cell::face_point(const myPoint& p) {
  Vector to_point = p.SubtractPointFromPoint(&part_loc);
  if (to_point.get_size() <= 0.001f) {
    return;
  }
  to_point.make_unit();
  sight_dir = to_point;
}

void Cell::see(World& world) {
  awareness.clear();
  grasp_space.clear();
  for (const auto& object : world.objects) {
    Entity* entity = object.get();
    if (!entity || entity == this || !entity->alive) {
      continue;
    }

    Vector v_obj = entity->part_loc.SubtractPointFromPoint(&part_loc);
    float distance = v_obj.get_size();
    if (distance > sight_height || distance <= 0.001f) {
      continue;
    }

    Vector dir = v_obj;
    dir.make_unit();
    float dot = std::clamp(sight_dir.DotProduct(&dir), -1.0f, 1.0f);
    float phi = std::acos(dot) * 180.0f / 3.14159f;
    if (phi > sight_angle) {
      continue;
    }

    PerceptualObject po;
    po.entity = entity;
    po.distance = distance;
    po.dir = dir;
    po.type = entity->type;
    po.moving = entity->vel.get_size() > 0.001f;
    awareness.push_back(po);
    if (distance <= radius + 10) {
      grasp_space.push_back(po);
    }
  }

  std::sort(awareness.begin(), awareness.end(), [](const PerceptualObject& a, const PerceptualObject& b) {
    return a.distance < b.distance;
  });
}

void Cell::try_eat(Entity* object) {
  if (!object || object->type != "food") {
    return;
  }

  auto* food = static_cast<Food*>(object);
  energy += food->energy;
  food->energy = 0;
  food->alive = false;
  hungry = false;
  feed_mode = false;
  explore = false;
  bored = true;
}

void Cell::feel_barrier() {
  hit_barrier = true;
  clear_thinking();
}

void Cell::clear_thinking() {
  pursuing_food = false;
  food_within_grasp = false;
  explore = true;
  feed_mode = false;
  turn_maneuver = false;
  busy = false;
}

void Cell::clear_thinkingB() {
  pursuing_food = false;
  food_within_grasp = false;
  explore = false;
  feed_mode = false;
}

void Cell::turn_shift(int ticks) {
  force_stop();
  turn_maneuver = true;
  busy = true;
  response_ticks = 0;
  end_tick = ticks;
}

void Cell::mind_set(World& world) {
  const Vector future_dx = vel * 16.65f;
  const myPoint future_point = part_loc.AddVectorToPoint(&future_dx);
  const SimPoint future_view{
    static_cast<int>(future_point.tuple[0]),
    static_cast<int>(future_point.tuple[1]),
  };

  if (!PtInRectPortable(&world.brc, future_view)) {
    clear_thinkingB();
    turn_shift(20);
  }

  if (hungry) {
    feed_mode = true;
    explore = true;
  } else if (!busy) {
    bored = true;
    explore = true;
  }
}

void Cell::simulate(World& world, float dt) {
  energy_depletion(-0.5f * dt);
  see(world);
  mind_set(world);

  if (!pursuing_food && !food_within_grasp) {
    if (hit_barrier && !responding_to_barrier) {
      rotate_eyes(110);
      hit_barrier = false;
      responding_to_barrier = true;
    } else if (responding_to_barrier) {
      hit_barrier = false;
      response_ticks++;
      if (response_ticks >= 50) {
        response_ticks = 0;
        responding_to_barrier = false;
      }
    }
  }

  if (pursuing_food) {
    Vector vfood = goal_point.SubtractPointFromPoint(&part_loc);
    float dist_to_food = vfood.get_size();

    for (const auto& obj : awareness) {
      if (obj.type == "food" && obj.distance < dist_to_food) {
        force_stop();
        face_point(obj.entity->part_loc);
        goal_point = obj.entity->part_loc;
        break;
      }
    }

    move_forward(100);
    if (dist_to_food <= radius + 10) {
      force_stop();
      food_within_grasp = true;
      pursuing_food = false;
    }
  } else if (food_within_grasp) {
    force_stop();
    bool ate_food = false;
    for (const auto& obj : grasp_space) {
      if (obj.type == "food") {
        face_point(obj.entity->part_loc);
        try_eat(obj.entity);
        ate_food = true;
      }
    }
    food_within_grasp = false;
    if (!ate_food) {
      explore = true;
    }
  } else {
    if (feed_mode) {
      for (const auto& obj : awareness) {
        if (obj.type == "food") {
          force_stop();
          face_point(obj.entity->part_loc);
          goal_point = obj.entity->part_loc;
          pursuing_food = true;
          turn_maneuver = false;
          break;
        }
      }
    }

    if (explore) {
      if (turn_maneuver) {
        rotate_eyes(4);
        move_forward(120);
        response_ticks++;
        if (response_ticks >= end_tick) {
          response_ticks = 0;
          turn_maneuver = false;
          busy = false;
        }
      } else {
        wander_turn_cooldown -= dt;
        if (wander_turn_cooldown <= 0) {
          rotate_eyes(35);
          wander_turn_cooldown = 3.0f;
        }
        move_forward(100);
        bored = true;
      }
    } else if (bored) {
      explore = true;
    }
  }

  energy_depletion(-2.0f);
}
