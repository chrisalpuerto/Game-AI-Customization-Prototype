#pragma once

#include "particle.h"
#include "platform_types.h"

#include <string>

class Entity : public particle {
public:
  std::string type;
  bool alive = true;
  SimPoint view_loc;

  virtual ~Entity() = default;
  virtual void tick(float dt) = 0;

  void sync_view_loc() {
    view_loc.x = static_cast<int>(part_loc.tuple[0]);
    view_loc.y = static_cast<int>(part_loc.tuple[1]);
  }
};

