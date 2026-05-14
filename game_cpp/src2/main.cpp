#include "world.h"

#include <iostream>

int main() {
  World world(8, 8, 1280, 720, 0.1f);

  world.spawn_cell({220, 220}, 1);
  world.spawn_cell({980, 520}, 2);
  world.spawn_food({420, 300}, true);
  world.spawn_food({780, 410}, false);
  world.spawn_food({630, 180}, false);
  world.spawn_linear_barrier({300, 500}, 0, 0, 0, 500);

  for (int i = 0; i < 600; i++) {
    world.simulate_world(0.1f);
    if (i % 60 == 0) {
      WorldSnapshot snap = world.snapshot();
      std::cout << "t=" << snap.time << " cells=" << snap.cells.size() << " food=" << snap.food.size() << "\n";
      for (size_t idx = 0; idx < snap.cells.size(); idx++) {
        const auto& cell = snap.cells[idx];
        std::cout << "  cell[" << idx << "] x=" << cell.x << " y=" << cell.y << " vx=" << cell.vx
                  << " vy=" << cell.vy << "\n";
      }
    }
  }

  return 0;
}

