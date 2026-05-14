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

void write_string_list(std::ostringstream& out, const std::vector<std::string>& values) {
  out << "[";
  for (size_t i = 0; i < values.size(); i++) {
    if (i > 0) {
      out << ",";
    }
    out << "\"" << json_escape(values[i]) << "\"";
  }
  out << "]";
}

void write_push_db(std::ostringstream& out, const std::vector<PushabilitySnapshot>& rows) {
  out << "[";
  for (size_t i = 0; i < rows.size(); i++) {
    const auto& row = rows[i];
    if (i > 0) {
      out << ",";
    }
    out << "{"
        << "\"objectType\":\"" << json_escape(row.objectType) << "\","
        << "\"pushability\":" << row.pushability << ","
        << "\"confidence\":" << row.confidence << ","
        << "\"numTrials\":" << row.numTrials
        << "}";
  }
  out << "]";
}

void write_conclusions(std::ostringstream& out, const std::vector<ConclusionSnapshot>& conclusions) {
  out << "[";
  for (size_t i = 0; i < conclusions.size(); i++) {
    const auto& conclusion = conclusions[i];
    if (i > 0) {
      out << ",";
    }
    out << "{"
        << "\"actionName\":\"" << json_escape(conclusion.actionName) << "\","
        << "\"objectType\":\"" << json_escape(conclusion.objectType) << "\","
        << "\"subject\":\"" << json_escape(conclusion.subject) << "\","
        << "\"result\":";
    write_string_list(out, conclusion.result);
    out << "}";
  }
  out << "]";
}

void write_cells(std::ostringstream& out, const std::vector<CellSnapshot>& cells) {
  out << "[";
  for (size_t i = 0; i < cells.size(); i++) {
    const auto& cell = cells[i];
    if (i > 0) {
      out << ",";
    }
    out << "{"
        << "\"id\":" << cell.id << ","
        << "\"type\":\"" << json_escape(cell.type) << "\","
        << "\"x\":" << cell.x << ","
        << "\"y\":" << cell.y << ","
        << "\"vx\":" << cell.vx << ","
        << "\"vy\":" << cell.vy << ","
        << "\"alive\":" << (cell.alive ? "true" : "false") << ","
        << "\"energy\":" << cell.energy << ","
        << "\"hungry\":" << (cell.hungry ? "true" : "false") << ","
        << "\"feedMode\":" << (cell.feedMode ? "true" : "false") << ","
        << "\"bored\":" << (cell.bored ? "true" : "false") << ","
        << "\"explore\":" << (cell.explore ? "true" : "false") << ","
        << "\"busy\":" << (cell.busy ? "true" : "false") << ","
        << "\"pursuingFood\":" << (cell.pursuingFood ? "true" : "false") << ","
        << "\"foodWithinGrasp\":" << (cell.foodWithinGrasp ? "true" : "false") << ","
        << "\"pursuingPush\":" << (cell.pursuingPush ? "true" : "false") << ","
        << "\"pushWithinGrasp\":" << (cell.pushWithinGrasp ? "true" : "false") << ","
        << "\"doObserve\":" << (cell.doObserve ? "true" : "false") << ","
        << "\"analyzeObservation\":" << (cell.analyzeObservation ? "true" : "false") << ","
        << "\"confused\":" << (cell.confused ? "true" : "false") << ","
        << "\"awarenessCount\":" << cell.awarenessCount << ","
        << "\"graspCount\":" << cell.graspCount << ","
        << "\"responseTicks\":" << cell.responseTicks << ","
        << "\"endTick\":" << cell.endTick << ","
        << "\"ooiType\":\"" << json_escape(cell.ooiType) << "\","
        << "\"pushGoalType\":\"" << json_escape(cell.pushGoalType) << "\","
        << "\"mode\":\"" << json_escape(cell.mode) << "\","
        << "\"pushDb\":";
    write_push_db(out, cell.pushDb);
    out << ",\"recentConclusions\":";
    write_conclusions(out, cell.recentConclusions);
    out << "}";
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
  write_cells(out, snapshot.cells);
  out << ",\"food\":";
  write_entities(out, snapshot.food);
  out << ",\"barriers\":";
  write_entities(out, snapshot.barriers);
  out << "}}";
  return out.str();
}
