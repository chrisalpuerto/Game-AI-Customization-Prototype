#include "particle.h"

#include <algorithm>
#include <cmath>

void particle::move(float dt) {
  Vector dx = vel.MultiplyVectorByScalar(dt);
  part_loc = part_loc.AddVectorToPoint(&dx);
}

void particle::force_stop() {
  vel = Vector(0, 0, 0);
}

void particle::accelerate(float dt) {
  Vector dacc = acceleration.MultiplyVectorByScalar(dt);
  vel = vel.AddVectorToVector(&dacc);
  acceleration = Vector(0, 0, 0);

  for (size_t i = 0; i < attachments.size(); i++) {
    Vector dx = attachments[i].pt.SubtractPointFromPoint(&part_loc);
    float distance = dx.get_size();
    if (distance >= attachments[i].length) {
      Vector R_vec = dx;
      R_vec.make_unit();
      Vector neg_R = R_vec * -1.0f;
      float vopp = vel.DotProduct(&neg_R);
      Vector vel_opp = neg_R * vopp;
      vel = vel - vel_opp;
    }
  }
}

void particle::applyForce(Vector force) {
  if (vel.get_size() >= max_speed) {
    return;
  }

  float rem_force = 0;
  std::vector<int> active_attachments;

  for (size_t i = 0; i < attachments.size(); i++) {
    Vector dx = attachments[i].pt.SubtractPointFromPoint(&part_loc);
    float distance = dx.get_size();

    if (distance >= attachments[i].length) {
      Vector R_vec = dx;
      R_vec.make_unit();
      Vector neg_R = R_vec.MultiplyVectorByScalar(-1);
      float F_opp = force.DotProduct(&neg_R);

      if (F_opp > 0) {
        float diff = attachments[i].strength - F_opp;
        if (diff >= 0) {
          Vector fres = R_vec.MultiplyVectorByScalar(F_opp);
          force += fres;
          float vopp = vel.DotProduct(&neg_R);
          Vector vel_opp = neg_R * vopp;
          vel = vel - vel_opp;
          rem_force = 0;
          break;
        }

        Vector fres = R_vec.MultiplyVectorByScalar(attachments[i].strength);
        force += fres;
        rem_force = -diff;
        active_attachments.push_back(static_cast<int>(i));
      }
    }
  }

  if (rem_force > 0) {
    for (int idx = static_cast<int>(active_attachments.size()) - 1; idx >= 0; idx--) {
      attachments.erase(attachments.begin() + active_attachments[idx]);
    }
  }

  Vector da = force.DivideVectorByScalar(std::max(mass, 0.001f));
  acceleration = acceleration.AddVectorToVector(&da);
  if (static_cast<int>(acceleration.get_size() * 1000) == 0) {
    acceleration = acceleration * 0;
  }
}

void particle::Collision(float mass_collide, float speed_x_collide, float speed_y_collide) {
  if (ignore_collision) {
    return;
  }

  float final_speed_x = (vel.tuple[0] * mass + speed_x_collide * mass_collide -
                         mass_collide * (vel.tuple[0] - speed_x_collide)) /
                        (mass_collide + mass);
  float final_speed_y = (vel.tuple[1] * mass + speed_y_collide * mass_collide -
                         mass_collide * (vel.tuple[1] - speed_y_collide)) /
                        (mass_collide + mass);

  vel.tuple[0] = final_speed_x;
  vel.tuple[1] = final_speed_y;
}

void particle::applyGravity(particle b) {
  if (gravity_exception) {
    return;
  }

  float G = 200000.0f;
  Vector disp = b.part_loc.SubtractPointFromPoint(&part_loc);
  float distance = std::max(disp.get_size(), 0.001f);
  disp = disp.DivideVectorByScalar(distance);
  float f = G * mass * b.mass / (distance * distance);
  float acc = f / std::max(mass, 0.001f);
  Vector a = disp.MultiplyVectorByScalar(acc);
  acceleration = acceleration.AddVectorToVector(&a);
}

void particle::applyElectricity(particle b) {
  if (ignore_electricity) {
    return;
  }

  Vector disp = b.part_loc.SubtractPointFromPoint(&part_loc);
  float distance = std::max(disp.get_size(), 0.001f);
  disp = disp.DivideVectorByScalar(distance);
  if (distance < (radius + b.radius)) {
    float charge_product = charge * b.charge;
    if (charge_product > 0) {
      float E = gentle_electric_mode ? 1.0f : 100.0f;
      float a_scalar = E * charge_product / (distance * distance) / std::max(mass, 0.001f);
      Vector a = disp.MultiplyVectorByScalar(a_scalar);

      if (gentle_electric_mode) {
        vel = Vector(0, 0, 0);
      }
      acceleration = acceleration.AddVectorToVector(&a);
    }
  }
}

void particle::attach_to_point(myPoint pt, float str, float length) {
  attachment nAtt;
  nAtt.pt = pt;
  nAtt.strength = str;
  nAtt.length = length;
  attachments.push_back(nAtt);
}

bool particle::inParticle(myPoint pt) {
  Vector disp = pt.SubtractPointFromPoint(&part_loc);
  return disp.get_size() <= radius;
}

