#include "cell.h"

#include "food.h"
#include "myMatrix.h"
#include "world.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kPi = 3.14159f;

}

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
  max_force = 1000;
  energy = 10000;
  hungry = false;

  feed_mode = false;
  pursuing_food = false;
  food_within_grasp = false;
  goal_point = myPoint(0, 0, 0);
  explore = true;
  hit_barrier = false;
  responding_to_barrier = false;
  response_ticks = 0;
  end_tick = 0;
  turn_maneuver = false;
  near_barrier = false;
  see_barrier = false;
  keep_distance_to_barrier = true;

  bored = true;
  pursuing_push = false;
  push_within_grasp = false;
  confused = false;

  do_observe = false;
  ooi = nullptr;
  ooi_type_label.clear();
  analyze_observation = false;
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

void Cell::force_self(Vector force) {
  energy_depletion(-1);
  applyForce(force);
}

void Cell::stop() {
  force_stop();
}

void Cell::face_point(const myPoint& p) {
  Vector stri_h = sight_dir.MultiplyVectorByScalar(static_cast<float>(sight_height));
  Vector v_obj = p.SubtractPointFromPoint(&part_loc);
  v_obj.tuple[1] *= -1;
  stri_h.tuple[1] *= -1;

  float dist_to_obj = v_obj.get_size();
  if (dist_to_obj <= 0.001f) {
    return;
  }

  float dot = v_obj.DotProduct(&stri_h);
  float cos_phi = dot / static_cast<float>(sight_height) / dist_to_obj;
  cos_phi = std::clamp(cos_phi, -1.0f, 1.0f);
  if (std::abs(1.0f - cos_phi) < 0.0001f) {
    cos_phi = 1.0f;
  }
  float phi = std::acos(cos_phi) * 180.0f / kPi;

  float alpha_vobj = v_obj.get_angle();
  float alpha_vsight = stri_h.get_angle();
  float diff = (alpha_vsight - alpha_vobj) * 180.0f / kPi;
  if (static_cast<int>(std::abs(diff) * 10) <= static_cast<int>(phi * 10)) {
    if (diff > 0) {
      phi *= -1;
    }
  } else if (diff < 0) {
    phi *= -1;
  }

  rotate_eyes(-1.0f * phi);
}

void Cell::turn_shift(int ticks) {
  stop();
  turn_maneuver = true;
  busy = true;
  response_ticks = 0;
  end_tick = ticks;
}

void Cell::try_eat(Entity* object) {
  if (!object || object->type != "food") {
    return;
  }

  auto* food = static_cast<Food*>(object);
  energy += food->energy;
  food->energy = 0;
  food->alive = false;

  float prop = energy / 15000.0f;
  if (prop >= 1.0f) {
    hungry = false;
    feed_mode = false;
    explore = false;
    bored = true;
  }
}

void Cell::try_push(Entity* object) {
  for (const auto& item : grasp_space) {
    if (object == item.entity) {
      float effort = 1000;
      Vector f = sight_dir;
      f = f.MultiplyVectorByScalar(effort);
      object->applyForce(f);
      push_within_grasp = false;
      break;
    }
  }
}

void Cell::observe_object(Entity* object) {
  for (const auto& item : awareness) {
    if (item.entity == object) {
      observations.push_back(item);
    }
  }
}

void Cell::feel_barrier() {
  clear_thinking();
}

void Cell::see(World& world) {
  clirect = world.brc;
  awareness.clear();
  grasp_space.clear();

  Vector stri_h = sight_dir.MultiplyVectorByScalar(static_cast<float>(sight_height));

  for (const auto& object : world.objects) {
    Entity* entity = object.get();
    if (!entity || entity == this || !entity->alive) {
      continue;
    }

    myPoint obj_point = entity->part_loc;
    Vector v_obj = obj_point.SubtractPointFromPoint(&part_loc);
    v_obj.tuple[1] *= -1;

    bool angle_within = false;
    bool distance_within_awareness = false;
    bool distance_within_grasp = false;

    float dist_to_obj = v_obj.get_size();
    if (dist_to_obj <= 0.001f) {
      continue;
    }

    Vector temp_stri_h = stri_h;
    temp_stri_h.tuple[1] *= -1;
    float dot = v_obj.DotProduct(&temp_stri_h);
    float cos_arg = dot / static_cast<float>(sight_height) / dist_to_obj;
    cos_arg = std::clamp(cos_arg, -1.0f, 1.0f);
    if (std::abs(1.0f - cos_arg) < 0.0001f) {
      cos_arg = 1.0f;
    }
    float phi = std::acos(cos_arg) * 180.0f / kPi;

    if (phi <= sight_angle) {
      angle_within = true;
    }

    if (dist_to_obj <= sight_height) {
      distance_within_awareness = true;
      if (dist_to_obj <= radius + 10) {
        distance_within_grasp = true;
      }
    }

    if (angle_within && distance_within_awareness) {
      PerceptualObject po;
      po.entity = entity;
      po.distance = dist_to_obj;
      po.dir = v_obj;
      po.dir.make_unit();
      po.type = entity->type;
      po.itsDir = entity->vel;
      po.itsDir.make_unit();
      po.itsSpeed = entity->vel.get_size();
      if (static_cast<int>(po.itsSpeed * 1000) > 0) {
        po.moving = true;
      }

      awareness.push_back(po);
      if (distance_within_grasp) {
        grasp_space.push_back(po);
      }
    }
  }

  std::sort(awareness.begin(), awareness.end(), [](const PerceptualObject& a, const PerceptualObject& b) {
    return a.distance < b.distance;
  });
  std::sort(grasp_space.begin(), grasp_space.end(), [](const PerceptualObject& a, const PerceptualObject& b) {
    return a.distance < b.distance;
  });

  const myPoint sight_tip = part_loc.AddVectorToPoint(&stri_h);
  const SimPoint tip{
    static_cast<int>(sight_tip.tuple[0]),
    static_cast<int>(sight_tip.tuple[1]),
  };

  if (outside_rect(tip, clirect)) {
    near_barrier = true;
    see_barrier = true;
  } else {
    near_barrier = false;
    see_barrier = false;
  }

  energy_depletion(-2);
}

void Cell::clear_thinking() {
  pursuing_food = false;
  pursuing_push = false;
  food_within_grasp = false;
  push_within_grasp = false;
  explore = true;
  confused = false;
  bored = false;
  do_observe = false;
  analyze_observation = false;
  observations.clear();
  ooi = nullptr;
  ooi_type_label.clear();
}

void Cell::clear_thinkingB() {
  pursuing_food = false;
  pursuing_push = false;
  food_within_grasp = false;
  push_within_grasp = false;
  explore = false;
  confused = false;
  bored = false;
  do_observe = false;
  analyze_observation = false;
  observations.clear();
  ooi = nullptr;
  ooi_type_label.clear();
}

void Cell::mind_set(World& world) {
  if (see_barrier && keep_distance_to_barrier) {
    clear_thinkingB();

    Vector dxdt = vel;
    Vector dx = dxdt * 16.65f;
    const myPoint projected = part_loc.AddVectorToPoint(&dx);
    const SimPoint future_view{
      static_cast<int>(projected.tuple[0]),
      static_cast<int>(projected.tuple[1]),
    };

    if (outside_rect(future_view, world.brc)) {
      turn_shift(20);
    }
  }

  if (hungry) {
    feed_mode = true;
    explore = true;
  } else if (!busy) {
    bored = true;
    explore = true;
  }
}

void Cell::frame_push_curious() {
  for (const auto& item : awareness) {
    if (!in_push_db(item.type)) {
      set_push_target(item);
    } else {
      for (const auto& summary : push_db) {
        if (summary.object_type == item.type && summary.confidence < 0.7f) {
          set_push_target(item);
          break;
        }
      }
    }
    if (pursuing_push) {
      break;
    }
  }
}

void Cell::frame_push_optimize() {
  int best_push_idx = -1;
  float best_rank = 0;
  for (size_t i = 0; i < push_db.size(); i++) {
    if (push_db[i].pushability > best_rank) {
      best_rank = push_db[i].pushability;
      best_push_idx = static_cast<int>(i);
    }
  }

  for (const auto& item : awareness) {
    if (best_push_idx == -1) {
      set_push_target(item);
      break;
    }
    if (item.type == push_db[best_push_idx].object_type) {
      set_push_target(item);
      break;
    }
  }
  if (!pursuing_push) {
    for (const auto& item : awareness) {
      set_push_target(item);
      break;
    }
  }
}

void Cell::set_push_target(const PerceptualObject& po) {
  stop();
  face_point(po.entity->part_loc);
  goal_point = po.entity->part_loc;
  push_goal = po;
  pursuing_push = true;
  turn_maneuver = false;
  explore = false;
}

void Cell::analyze() {
  const std::string observed_type = !ooi_type_label.empty() ? ooi_type_label : (ooi ? ooi->type : "");
  if (observed_type.empty() || observations.size() < 2) {
    observations.clear();
    return;
  }

  std::vector<AnalysisObject> analysis_list;
  for (size_t i = 0; i + 1 < observations.size(); i++) {
    AnalysisObject a1;
    const PerceptualObject& before = observations[i];
    const PerceptualObject& after = observations[i + 1];

    if (before.moving != after.moving) {
      a1.changed_moving = true;
      if (before.moving && !after.moving) {
        a1.stopped = true;
      } else {
        a1.moved_it = true;
      }
    }

    int delta_dist = static_cast<int>((after.distance - before.distance) * 1000);
    if (delta_dist > 0) {
      a1.got_further = true;
    }
    if (delta_dist < 0) {
      a1.got_closer = true;
    }

    int delta_speed = static_cast<int>((after.itsSpeed - before.itsSpeed) * 1000);
    if (delta_speed > 0) {
      a1.sped_up = true;
    }
    if (delta_speed < 0) {
      a1.slowed_down = true;
    }
    analysis_list.push_back(a1);
  }

  int count_further = 0;
  int count_closer = 0;
  int count_no_change = 0;
  bool event_caused_movement = false;
  bool object_reached_stop = false;

  for (const auto& item : analysis_list) {
    if (item.moved_it) {
      event_caused_movement = true;
    }
    if (event_caused_movement) {
      if (item.got_further) count_further++;
      if (item.got_closer) count_closer++;
    }
    if (item.stopped) {
      object_reached_stop = true;
    }
    if (!item.changed_moving && !item.got_closer && !item.got_further && !item.slowed_down && !item.sped_up) {
      count_no_change++;
    }
  }

  const float num_obs = static_cast<float>(analysis_list.size());
  if (num_obs <= 0.0f) {
    observations.clear();
    return;
  }

  float prop_further = static_cast<float>(count_further) / num_obs;

  bool ps_entry_exists = false;
  for (auto& summary : push_db) {
    if (observed_type == summary.object_type) {
      ps_entry_exists = true;
      float nPush = (summary.num_trials * summary.pushability + prop_further) / (summary.num_trials + 1);
      summary.pushability = nPush;
      summary.num_trials++;
      summary.confidence = static_cast<float>(summary.num_trials) / 100.0f;
    }
  }
  if (!ps_entry_exists) {
    PushabilitySummary ps;
    ps.object_type = observed_type;
    ps.num_trials = 1;
    ps.pushability = prop_further;
    ps.confidence = 0.01f;
    push_db.push_back(ps);
  }

  Conclusion con;
  con.subject = "I";
  con.action_type = 0;
  con.action_name = "push";
  con.object_type = observed_type;

  if (event_caused_movement) {
    con.result.push_back("to move");
  }
  if (object_reached_stop) {
    con.result.push_back("to stop");
  }
  if (!event_caused_movement && !object_reached_stop) {
    con.result.push_back("no result");
  }
  conclusions.push_back(con);
  if (conclusions.size() > 24) {
    conclusions.erase(conclusions.begin(), conclusions.end() - 24);
  }

  observations.clear();
}

bool Cell::in_push_db(const std::string& item) const {
  for (const auto& summary : push_db) {
    if (item == summary.object_type) {
      return true;
    }
  }
  return false;
}

void Cell::think() {
  if (confused) {
    stop();
    clear_thinking();
  }

  if (!pursuing_food && !food_within_grasp && !pursuing_push && !push_within_grasp) {
    if (hit_barrier && !responding_to_barrier) {
      rotate_eyes(110);
      hit_barrier = false;
      responding_to_barrier = true;
    } else if (responding_to_barrier) {
      hit_barrier = false;
      response_ticks++;
      if (response_ticks == 50) {
        response_ticks = 0;
        responding_to_barrier = false;
      }
    } else if (near_barrier) {
      turn_shift(20);
    }
  }

  if (pursuing_food) {
    Vector vfood = goal_point.SubtractPointFromPoint(&part_loc);
    float dist_to_food = vfood.get_size();

    for (const auto& item : awareness) {
      if (item.type == "food" && item.distance < dist_to_food) {
        stop();
        face_point(item.entity->part_loc);
        goal_point = item.entity->part_loc;
        pursuing_food = true;
        break;
      }
    }

    move_forward(100);
    if (dist_to_food <= radius + 10) {
      stop();
      food_within_grasp = true;
      pursuing_food = false;
    }
  } else if (food_within_grasp) {
    stop();

    for (const auto& item : grasp_space) {
      if (item.type == "food") {
        face_point(item.entity->part_loc);
        try_eat(item.entity);
      }
    }
    food_within_grasp = false;
  } else if (pursuing_push) {
    bool match_found = false;
    float search_radius = 10;

    for (const auto& item : awareness) {
      if (item.type != push_goal.type) {
        continue;
      }

      Vector dx = item.entity->part_loc.SubtractPointFromPoint(&goal_point);
      if (dx.get_size() <= search_radius) {
        goal_point = item.entity->part_loc;
        push_goal = item;
        match_found = true;
        break;
      }
    }

    if (!match_found) {
      confused = true;
    }

    face_point(goal_point);
    if (push_goal.moving) {
      move_forward(300);
    } else {
      move_forward(100);
    }
    if (push_goal.distance <= radius + 20) {
      push_within_grasp = true;
      pursuing_push = false;
    }
  } else if (push_within_grasp) {
    for (const auto& item : grasp_space) {
      if (item.type == push_goal.type) {
        face_point(item.entity->part_loc);
        observe_object(item.entity);
        do_observe = true;
        bored = false;
        ooi = item.entity;
        ooi_type_label = item.type;
        response_ticks = 0;
        end_tick = 30;
        try_push(item.entity);
        break;
      }
    }
    push_within_grasp = false;
  } else {
    if (hungry) {
      feed_mode = true;
      explore = true;
    }

    if (bored) {
      frame_push_curious();
    }
    if (feed_mode) {
      for (const auto& item : awareness) {
        if (item.type == "food") {
          stop();
          face_point(item.entity->part_loc);
          goal_point = item.entity->part_loc;
          pursuing_food = true;
          turn_maneuver = false;
          break;
        }
      }
    }

    if (do_observe && !near_barrier) {
      stop();
      if (ooi) {
        face_point(ooi->part_loc);
        observe_object(ooi);
      } else {
        confused = true;
      }
      response_ticks++;
      if (response_ticks == end_tick) {
        do_observe = false;
        analyze_observation = true;
      }
    } else if (analyze_observation) {
      analyze();
      analyze_observation = false;
    } else if (explore) {
      if (turn_maneuver) {
        rotate_eyes(4);
        move_forward(120);
        response_ticks++;
        if (response_ticks == 20) {
          response_ticks = 0;
          turn_maneuver = false;
          busy = false;
        }
      } else {
        move_forward(100);
        bored = true;
      }
    } else if (bored) {
      explore = true;
    } else {
      bored = true;
    }
  }

  energy_depletion(-2);
}

bool Cell::outside_rect(SimPoint pt, const SimRect& rect) const {
  return pt.x >= rect.right || pt.x <= rect.left || pt.y >= rect.bottom || pt.y <= rect.top;
}

std::string Cell::current_mode() const {
  if (analyze_observation) return "analyzing";
  if (do_observe) return "observing";
  if (pursuing_push || push_within_grasp) return "pushing";
  if (food_within_grasp) return "food_grasp";
  if (pursuing_food) return "seeking_food";
  if (turn_maneuver) return "turning";
  if (explore) return "exploring";
  return "idle";
}

void Cell::simulate(World& world, float) {
  clirect = world.brc;
  see(world);
  think();
}
