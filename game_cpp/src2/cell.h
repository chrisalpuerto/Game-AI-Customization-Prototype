#pragma once

#include "entity.h"

#include <string>
#include <vector>

class World;

struct PerceptualObject {
  Entity* entity = nullptr;
  float distance = 0;
  Vector dir;
  std::string type;
  bool moving = false;
  Vector itsDir;
  float itsSpeed = 0;
};

struct AnalysisObject {
  bool changed_moving = false;
  bool stopped = false;
  bool moved_it = false;
  bool got_further = false;
  bool got_closer = false;
  bool sped_up = false;
  bool slowed_down = false;
};

struct Conclusion {
  int action_type = -1;
  std::string action_name;
  std::string object_type;
  std::string subject;
  std::vector<std::string> result;
};

struct PushabilitySummary {
  std::string object_type;
  float pushability = 0;
  float confidence = 0;
  int num_trials = 0;
};

class Cell : public Entity {
public:
  SimRect clirect;
  int sight_height = 500;
  float sight_angle = 50;
  Vector sight_dir;
  float energy = 10000;
  bool hungry = false;
  bool bored = true;
  float max_force = 1000;
  int cell_id = 0;
  std::vector<PerceptualObject> awareness;
  std::vector<PerceptualObject> grasp_space;
  std::vector<PerceptualObject> observations;
  std::vector<Conclusion> conclusions;
  std::vector<PushabilitySummary> push_db;
  bool feed_mode = false;
  bool explore = true;
  bool pursuing_food = false;
  bool food_within_grasp = false;
  bool hit_barrier = false;
  bool responding_to_barrier = false;
  bool turn_maneuver = false;
  bool near_barrier = false;
  bool see_barrier = false;
  bool keep_distance_to_barrier = true;
  bool busy = false;
  int response_ticks = 0;
  int end_tick = 0;
  bool pursuing_push = false;
  PerceptualObject push_goal;
  bool push_within_grasp = false;
  bool confused = false;
  Entity* ooi = nullptr;
  std::string ooi_type_label;
  bool do_observe = false;
  bool analyze_observation = false;
  bool pursuing_goal_point = false;
  bool goal_obj_within_grasp = false;
  myPoint goal_point;
  PerceptualObject pursuit_goal_perception;

  Cell(SimPoint loc, int _id);
  ~Cell();

  void tick(float dt) override;
  void simulate(World& world, float dt);
  void force_self(Vector force);
  void stop();
  void move_forward(float force);
  void face_point(const myPoint& p);
  void rotate_eyes(float angle);
  void energy_depletion(float delta);
  void see(World& world);
  void try_eat(Entity* object);
  void try_push(Entity* object);
  void observe_object(Entity* object);
  void feel_barrier();
  void think();
  void analyze();
  bool in_push_db(const std::string& item) const;
  void clear_thinking();
  void clear_thinkingB();
  void turn_shift(int ticks);
  void mind_set(World& world);
  void frame_push_curious();
  void frame_push_optimize();
  void set_push_target(const PerceptualObject& po);
  bool outside_rect(SimPoint pt, const SimRect& rect) const;
  std::string current_mode() const;
};
