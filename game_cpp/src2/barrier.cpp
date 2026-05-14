#include "barrier.h"

#include <cmath>

Barrier::Barrier(std::vector<float> coeff, int degree_in, float xmin_in, float xmax_in, float precision_in, int padding_in,
                 SimPoint origin) {
  coefficients = std::move(coeff);
  degree = degree_in;
  xmin = xmin_in;
  xmax = xmax_in;
  precision = precision_in;
  padding = padding_in;
  type = "barrier";
  view_loc = origin;
  part_loc = myPoint(static_cast<float>(origin.x), static_cast<float>(origin.y), 0);
  ignore_collision = true;
  max_speed = 0;
}

Barrier::~Barrier() {}

float Barrier::f_of_x(float x) const {
  float y = 0;
  for (int i = 0; i <= degree; i++) {
    y += coefficients[i] * std::pow(x, i);
  }
  return y;
}

float Barrier::getDerivative(float x) const {
  float dydx = 0;
  for (int i = 1; i <= degree; i++) {
    dydx += i * coefficients[i] * std::pow(x, i - 1);
  }
  return dydx;
}

bool Barrier::isCollision(SimPoint test_pt) const {
  float local_x = static_cast<float>(test_pt.x - view_loc.x);
  if (local_x < xmin || local_x > xmax) {
    return false;
  }

  float curve_y = f_of_x(local_x);
  float local_y = static_cast<float>(test_pt.y - view_loc.y);
  return std::abs(local_y - curve_y) <= padding;
}

void Barrier::tick(float) {
  sync_view_loc();
}

