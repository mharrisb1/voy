#pragma once

#include <voy/event.hpp>

#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace voy::config {

using NativeCallback = std::function<void(const std::vector<event::Event>&)>;

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
  NativeCallback           callback;
};

struct VoyConfig {
  uint8_t                  version{1u};
  uint32_t                 debounce_ms{150};
  std::vector<RouteConfig> routes;
};

}  // namespace voy::config
