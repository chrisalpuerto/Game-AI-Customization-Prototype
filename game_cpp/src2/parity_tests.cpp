#include "barrier.h"
#include "cell.h"
#include "land.h"
#include "snapshot_json.h"
#include "world.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool approx(float a, float b, float epsilon = 0.01f) {
  return std::fabs(a - b) <= epsilon;
}

bool test_barrier_collision() {
  Barrier sloped({0.0f, 1.0f}, 1, 0.0f, 200.0f, 5.0f, 20, {0, 0});
  if (!sloped.isCollision({50, 60})) {
    std::cerr << "Expected sloped barrier collision near line y=x\n";
    return false;
  }
  if (sloped.isCollision({50, 120})) {
    std::cerr << "Unexpected sloped barrier collision far from line y=x\n";
    return false;
  }

  Barrier curved({0.0f, 0.0f, 0.01f}, 2, 0.0f, 200.0f, 5.0f, 15, {0, 0});
  if (!curved.isCollision({40, 23})) {
    std::cerr << "Expected curved barrier collision near parabola\n";
    return false;
  }
  if (curved.isCollision({40, 60})) {
    std::cerr << "Unexpected curved barrier collision far from parabola\n";
    return false;
  }

  return true;
}

bool test_land_behavior() {
  land plot({0, 0}, 100, 100);
  plot.mid_temp = 50.0f;
  plot.temp_radius = 30.0f;
  plot.calculate_temperature(3.14159f / 2.0f);
  if (!approx(plot.temperature, 80.0f, 0.1f)) {
    std::cerr << "Land temperature does not match original sine timing\n";
    return false;
  }

  plot.nutrient_level = 250.0f;
  plot.moisture_level = 100.0f;
  plot.sunlight = 0.0f;
  plot.temperature = 0.0f;
  plot.calculate_fertility();
  if (!approx(plot.fertility, 0.75f, 0.001f)) {
    std::cerr << "Land fertility does not match nutrient/moisture average\n";
    return false;
  }

  World world(2, 2, 200, 200, 0.1f);
  world.rain.magnitude = 0.0f;
  RainBox full_world_rain;
  full_world_rain.magnitude = 10.0f;
  full_world_rain.rain_width = 200.0f;
  full_world_rain.rain_height = 200.0f;
  full_world_rain.init_rain(0, 0);
  world.rains.push_back(full_world_rain);
  const float before = world.land_plots.front().moisture_level;
  world.simulate_land();
  if (world.land_plots.front().moisture_level <= before) {
    std::cerr << "Supplemental rain boxes did not affect land moisture\n";
    return false;
  }

  return true;
}

bool test_cell_food_seek_and_eat() {
  World world(8, 8, 1280, 720, 0.1f);
  Cell* cell = world.spawn_cell({200, 200}, 1);
  Food* farther_food = world.spawn_food({420, 200}, false);
  Food* nearer_food = world.spawn_food({280, 200}, false);

  cell->hungry = true;
  for (int i = 0; i < 200; i++) {
    world.simulate_world(0.1f);
    if (!nearer_food->alive) {
      break;
    }
  }

  if (nearer_food->alive) {
    std::cerr << "Hungry cell failed to eat nearer food\n";
    return false;
  }
  if (!farther_food->alive) {
    std::cerr << "Cell should prefer the nearer food before the farther one\n";
    return false;
  }

  return true;
}

bool test_cell_barrier_response() {
  World world(8, 8, 400, 300, 0.1f);
  Cell* cell = world.spawn_cell({390, 150}, 1);
  const float initial_heading_x = cell->sight_dir.tuple[0];
  for (int i = 0; i < 40; i++) {
    world.simulate_world(0.1f);
  }

  if (approx(cell->sight_dir.tuple[0], initial_heading_x, 0.01f)) {
    std::cerr << "Cell did not change heading near boundary as expected\n";
    return false;
  }
  if (!PtInRectPortable(&world.brc, cell->view_loc)) {
    std::cerr << "Cell left the world bounds during boundary response\n";
    return false;
  }

  return true;
}

bool test_cell_push_observe_learning() {
  World world(8, 8, 1280, 720, 0.1f);
  Cell* cell = world.spawn_cell({200, 200}, 1);
  Food* food = world.spawn_food({225, 200}, false);

  cell->energy = 16000.0f;
  cell->hungry = false;
  cell->bored = true;
  cell->feed_mode = false;
  cell->explore = true;

  bool saw_pushing = false;
  for (int i = 0; i < 80; i++) {
    world.simulate_world(0.1f);
    saw_pushing = saw_pushing || cell->pursuing_push || cell->push_within_grasp;
    if (saw_pushing) {
      break;
    }
  }

  if (!saw_pushing) {
    std::cerr << "Non-hungry cell never entered push behavior\n";
    return false;
  }

  cell->pursuing_push = false;
  cell->push_within_grasp = true;
  cell->explore = false;
  cell->bored = false;
  cell->push_goal.entity = food;
  cell->push_goal.type = "food";
  cell->push_goal.distance = 10.0f;

  bool saw_observing = false;
  bool saw_analyzing = false;
  for (int i = 0; i < 220; i++) {
    world.simulate_world(0.1f);
    saw_observing = saw_observing || cell->do_observe;
    saw_analyzing = saw_analyzing || cell->analyze_observation;
    if (!cell->push_db.empty() && !cell->conclusions.empty()) {
      break;
    }
  }

  if (!saw_observing) {
    std::cerr << "Cell never entered observe mode after push\n";
    return false;
  }
  if (!saw_analyzing && cell->conclusions.empty()) {
    std::cerr << "Cell never analyzed observations into conclusions\n";
    return false;
  }
  if (cell->push_db.empty()) {
    std::cerr << "Cell did not learn any pushability summary\n";
    return false;
  }
  if (cell->push_db.front().num_trials <= 0 || cell->push_db.front().confidence <= 0.0f) {
    std::cerr << "Push DB did not record valid trials/confidence\n";
    return false;
  }
  if (cell->conclusions.empty()) {
    std::cerr << "Cell did not produce conclusions from observations\n";
    return false;
  }

  return true;
}

bool test_snapshot_debug_fields() {
  World world(8, 8, 1280, 720, 0.1f);
  Cell* cell = world.spawn_cell({200, 200}, 1);
  cell->energy = 16000.0f;
  cell->hungry = false;
  cell->pursuing_push = true;
  cell->push_goal.type = "food";
  cell->ooi_type_label = "food";
  cell->do_observe = true;

  PushabilitySummary summary;
  summary.object_type = "food";
  summary.pushability = 0.75f;
  summary.confidence = 0.25f;
  summary.num_trials = 25;
  cell->push_db.push_back(summary);

  Conclusion conclusion;
  conclusion.action_name = "push";
  conclusion.object_type = "food";
  conclusion.subject = "I";
  conclusion.result.push_back("to move");
  cell->conclusions.push_back(conclusion);

  const WorldSnapshot snapshot = world.snapshot();
  if (snapshot.cells.empty()) {
    std::cerr << "Snapshot did not include cells\n";
    return false;
  }

  const auto& snap_cell = snapshot.cells.front();
  if (snap_cell.mode.empty() || snap_cell.pushDb.empty() || snap_cell.recentConclusions.empty()) {
    std::cerr << "Snapshot missing debug cognition fields\n";
    return false;
  }
  if (snap_cell.ooiType != "food" || snap_cell.pushGoalType != "food") {
    std::cerr << "Snapshot target labels are incorrect\n";
    return false;
  }

  const std::string json = snapshot_to_json(snapshot);
  if (json.find("\"mode\":\"") == std::string::npos ||
      json.find("\"pushDb\":") == std::string::npos ||
      json.find("\"recentConclusions\":") == std::string::npos) {
    std::cerr << "Snapshot JSON missing expected debug keys\n";
    return false;
  }

  return true;
}

}  // namespace

int main() {
  if (!test_barrier_collision()) return 1;
  if (!test_land_behavior()) return 1;
  if (!test_cell_food_seek_and_eat()) return 1;
  if (!test_cell_barrier_response()) return 1;
  if (!test_cell_push_observe_learning()) return 1;
  if (!test_snapshot_debug_fields()) return 1;

  std::cout << "All parity tests passed.\n";
  return 0;
}
