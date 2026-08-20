#include "config.hpp"

#include <voy/config.hpp>
#include <voy/event.hpp>

#include <expected>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

namespace voy::event {
void from_json(const nlohmann::json& j, EventType& e) {
  if (j.is_string()) {
    if (auto type = from_string(j.get<std::string>())) {
      e = *type;
      return;
    }
  } else if (j.is_array()) {
    auto strings = j.get<std::vector<std::string>>();
    if (auto type = from_strings(strings)) {
      e = *type;
      return;
    }
  }
  throw nlohmann::json::parse_error::create(0, 0, "Invalid EventType string or array", &j);
}
}  // namespace voy::event

namespace voy::config {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ActionConfig, command, workdir, env,
                                                pgroup_isolation, cooldown_ms)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RouteConfig, name, watch, ignore, events, action)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(VoyConfig, version, debounce_ms, routes)
}  // namespace voy::config

namespace voy::cli {

std::expected<voy::config::VoyConfig, std::string> parse_config_file(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) return std::unexpected("Could not open config file: " + path);

  nlohmann::json j;
  try {
    file >> j;

    // 3. One line to parse the entire config tree!
    auto config = j.get<voy::config::VoyConfig>();
    return config;

  } catch (const nlohmann::json::exception& e) {
    return std::unexpected("JSON error: " + std::string(e.what()));
  }
}

}  // namespace voy::cli
