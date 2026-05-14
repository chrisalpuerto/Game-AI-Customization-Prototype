#pragma once

#include "entity.h"

#include <vector>

class Barrier : public Entity {
public:
  std::vector<float> coefficients;
  int degree = 0;
  float precision = 1;
  float xmax = 0;
  float xmin = 0;
  int padding = 0;

  Barrier(std::vector<float> coeff, int degree_in, float xmin_in, float xmax_in, float precision_in, int padding_in,
          SimPoint origin);
  ~Barrier();

  bool isCollision(SimPoint test_pt) const;
  float getDerivative(float x) const;
  float f_of_x(float x) const;
  void tick(float dt) override;
};

