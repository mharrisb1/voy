#pragma once

#include <voy/config.hpp>
#include <voy/event.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace voy::pipeline {

class Pipeline {
 public:
  [[nodiscard]] static std::unordered_map<std::string, std::string> prepare_environment(
      const config::RouteConfig& route, std::vector<event::Event>& matched_events);
};

}  // namespace voy::pipeline
