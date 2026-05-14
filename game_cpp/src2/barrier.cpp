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
  SimPoint test_pt_prime;
  test_pt_prime.x = test_pt.x - view_loc.x;
  test_pt_prime.y = test_pt.y - view_loc.y;

  SimPoint curve_pt;
  bool found = false;

  const float prec = 0.1f;
  for (float x = xmin; x < xmax; x += prec) {
    float dydx = getDerivative(x);
    float y = f_of_x(x);

    float denom = y - static_cast<float>(test_pt_prime.y);
    if (std::abs(denom) <= 0.0001f) {
      continue;
    }

    float diff = dydx + (x - static_cast<float>(test_pt_prime.x)) / denom;
    if (std::abs(diff) < 0.1f) {
      curve_pt.x = static_cast<int>(x);
      curve_pt.y = static_cast<int>(y);
      found = true;
      break;
    }
  }

  if (!found) {
    return false;
  }

  float dx = static_cast<float>(curve_pt.x - test_pt_prime.x);
  float dy = static_cast<float>(curve_pt.y - test_pt_prime.y);
  float dist = std::sqrt(dx * dx + dy * dy);
  return dist <= static_cast<float>(padding);
}

void Barrier::tick(float) {
  sync_view_loc();
}
