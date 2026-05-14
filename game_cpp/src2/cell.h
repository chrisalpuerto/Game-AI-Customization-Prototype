#pragma once

#include "entity.h"

#include <vector>

class World;

struct PerceptualObject {
  Entity* entity = nullptr;
  float distance = 0;
  Vector dir;
  std::string type;
  bool moving = false;
};

class Cell : public Entity {
public:
  int sight_height = 500;
  float sight_angle = 50;
  Vector sight_dir;
  float energy = 10000;
  bool hungry = false;
  bool bored = true;
  float max_force = 1000;
  float wander_turn_cooldown = 0;
  int cell_id = 0;
  std::vector<PerceptualObject> awareness;
  std::vector<PerceptualObject> grasp_space;
  bool feed_mode = false;
  bool explore = true;
  bool pursuing_food = false;
  bool food_within_grasp = false;
  bool hit_barrier = false;
  bool responding_to_barrier = false;
  bool turn_maneuver = false;
  bool busy = false;
  int response_ticks = 0;
  int end_tick = 0;
  myPoint goal_point;

  Cell(SimPoint loc, int _id);
  ~Cell();

  void tick(float dt) override;
  void simulate(World& world, float dt);
  void move_forward(float force);
  void face_point(const myPoint& p);
  void rotate_eyes(float angle);
  void energy_depletion(float delta);
  void see(World& world);
  void try_eat(Entity* object);
  void feel_barrier();
  void clear_thinking();
  void clear_thinkingB();
  void turn_shift(int ticks);
  void mind_set(World& world);
};
