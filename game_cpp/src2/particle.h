#pragma once

#include "Vector.h"
#include "myPoint.h"

#include <vector>

struct attachment {
  myPoint pt;
  float strength = 0;
  float length = 0;
};

class particle {
public:
  int id = 0;
  int radius = 0;
  float mass = 1;
  myPoint part_loc;
  Vector vel;
  Vector acceleration;
  float charge = 0;
  unsigned int color = 0;
  bool attached = false;
  std::vector<attachment> attachments;
  float max_speed = 0;
  bool gravity_exception = false;
  bool ignore_collision = false;
  bool ignore_electricity = false;
  bool gentle_electric_mode = false;

  void move(float dt);
  void accelerate(float dt);
  void applyForce(Vector force);
  void Collision(float mass_collide, float speed_x_collide, float speed_y_collide);
  void applyGravity(particle b);
  void applyElectricity(particle b);
  bool inParticle(myPoint pt);
  void attach_to_point(myPoint pt, float str, float length);
  void force_stop();
};

