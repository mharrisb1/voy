#pragma once

#include "voy/event.hpp"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace voy::config {

struct ActionConfig {
  std::string                                  command;
  std::string                                  workdir{"."};
  std::unordered_map<std::string, std::string> env;
  bool                                         pgroup_isolation{true};
  uint32_t                                     cooldown_ms{500};
};

struct RouteConfig {
  std::string              name{"_anon"};
  std::vector<std::string> watch;
  std::vector<std::string> ignore;
  event::EventType         events{event::EventType::All};
  ActionConfig             action;
};

struct VoyConfig {
  uint8_t                  version{1u};
  uint32_t                 debounce_ms{150};
  std::vector<RouteConfig> routes;

  [[nodiscard]] static std::expected<VoyConfig, std::string> from_file(
      const std::filesystem::path& path);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ActionConfig, command, workdir, env,
                                                pgroup_isolation, cooldown_ms)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RouteConfig, name, watch, ignore, events, action)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(VoyConfig, version, debounce_ms, routes)

}  // namespace voy::config
