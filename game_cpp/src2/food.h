#pragma once

#include "entity.h"

class Food : public Entity {
public:
  float energy = 250;

  explicit Food(SimPoint loc);
  ~Food();

  void tick(float dt) override;
};

