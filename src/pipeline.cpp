/*
 * markings:managed
 *
 * File: pipeline.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#include <voy/event.hpp>
#include <voy/pipeline.hpp>

#include <format>
#include <ranges>
#include <unordered_map>

namespace voy::pipeline {

constexpr char PATH_SEP = ':';

std::unordered_map<std::string, std::string> Pipeline::prepare_environment(
    const config::RouteConfig& route, const std::vector<event::Event>& matched_events) {
  std::unordered_map<std::string, std::string> env = route.action.env;

  if (matched_events.empty()) return env;

  const auto batch_size = static_cast<long>(matched_events.size());

  env["VOY_ROUTE_NAME"] = route.name;
  env["VOY_BATCH_SIZE"] = std::to_string(batch_size);

  std::string      pathbuf;
  event::EventType typebuf = event::EventType::None;

  for (const auto& [ix, event] : std::views::enumerate(matched_events)) {
    pathbuf += event.path;
    typebuf |= event.type;
    if (ix == 0) env["VOY_EVENT_TIME"] = std::format("{}", event.timestamp);
    if (ix < batch_size - 1) pathbuf += PATH_SEP;
  }

  env["VOY_EVENT_PATH"] = pathbuf;
  env["VOY_EVENT_TYPE"] = event::to_composite_string(typebuf);

  return env;
}

}  // namespace voy::pipeline
