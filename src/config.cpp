#include <voy/config.hpp>
#include <voy/event.hpp>

#include <fstream>
#include <stdexcept>

namespace voy::config {

using json = nlohmann::json;

void from_json(const json& j, event::EventType& type) {
  if (j.is_array()) {
    std::vector<std::string> event_strings;
    j.get_to(event_strings);

    auto mask = event::from_strings(event_strings);
    if (!mask) throw std::runtime_error("Invalid event type configfured");
    type = *mask;
  }
}

std::expected<VoyConfig, std::string> VoyConfig::from_file(const std::filesystem::path& path) {
  std::ifstream file(path);
  if (!file.is_open()) { return std::unexpected("Failed to open file: " + path.string()); }

  try {
    return json::parse(file).get<VoyConfig>();
  } catch (const json::parse_error& e) {
    return std::unexpected(std::string("JSON parse error: ") + e.what());
  } catch (std::exception& e) {
    return std::unexpected(std::string("Config validation error: ") + e.what());
  }
}

}  // namespace voy::config
