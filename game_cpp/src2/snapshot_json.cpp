#include "snapshot_json.h"

#include <iomanip>
#include <sstream>

namespace {

std::string json_escape(const std::string& input) {
  std::ostringstream out;
  for (char c : input) {
    switch (c) {
      case '"':
        out << "\\\"";
        break;
      case '\\':
        out << "\\\\";
        break;
      case '\b':
        out << "\\b";
        break;
      case '\f':
        out << "\\f";
        break;
      case '\n':
        out << "\\n";
        break;
      case '\r':
        out << "\\r";
        break;
      case '\t':
        out << "\\t";
        break;
      default:
        out << c;
        break;
    }
  }
  return out.str();
}

void write_entities(std::ostringstream& out, const std::vector<EntitySnapshot>& entities) {
  out << "[";
  for (size_t i = 0; i < entities.size(); i++) {
    const auto& entity = entities[i];
    if (i > 0) {
      out << ",";
    }
    out << "{"
        << "\"id\":" << entity.id << ","
        << "\"type\":\"" << json_escape(entity.type) << "\","
        << "\"x\":" << entity.x << ","
        << "\"y\":" << entity.y << ","
        << "\"vx\":" << entity.vx << ","
        << "\"vy\":" << entity.vy << ","
        << "\"alive\":" << (entity.alive ? "true" : "false")
        << "}";
  }
  out << "]";
}

}  // namespace

std::string snapshot_to_json(const WorldSnapshot& snapshot) {
  std::ostringstream out;
  out << std::fixed << std::setprecision(3);
  out << "{"
      << "\"type\":\"snapshot\","
      << "\"time\":" << snapshot.time << ","
      << "\"world\":{"
      << "\"width\":" << snapshot.width << ","
      << "\"height\":" << snapshot.height << ","
      << "\"cells\":";
  write_entities(out, snapshot.cells);
  out << ",\"food\":";
  write_entities(out, snapshot.food);
  out << ",\"barriers\":";
  write_entities(out, snapshot.barriers);
  out << "}}";
  return out.str();
}

