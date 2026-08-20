#include <voy/event.hpp>
#include <voy/pipeline.hpp>

#include <format>

namespace voy::pipeline {

namespace fs = std::filesystem;

std::unordered_map<std::string, std::string> Pipeline::prepare_environment(
    const config::RouteConfig& route, const std::vector<event::Event>& matched_events) {
  std::unordered_map<std::string, std::string> env = route.action.env;

  if (matched_events.empty()) return env;

  // NOTE: Just taking the first event for now to hydrate env var params.
  //       Will need to reconsider in the future.
  const auto&     event    = matched_events[0];
  const fs::path& abs_path = event.path;

  env["VOY_ROUTE_NAME"] = route.name;
  env["VOY_BATCH_SIZE"] = std::to_string(matched_events.size());
  env["VOY_EVENT_TYPE"] = event::to_composite_string(event.type);
  env["VOY_EVENT_TIME"] = std::format("{}", event.timestamp);
  env["VOY_EVENT_PATH"] = abs_path;

  return env;
}

}  // namespace voy::pipeline
