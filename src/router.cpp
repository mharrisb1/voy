/*
 * markings:managed
 *
 * File: router.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#include <voy/config.hpp>
#include <voy/event.hpp>
#include <voy/glob.hpp>
#include <voy/router.hpp>

#include <filesystem>
#include <type_traits>
#include <utility>

namespace voy::router {

Router::Router(const config::VoyConfig& config) : rootdir_(config.rootdir) {
  compiled_routes_.reserve(config.routes.size());

  for (const auto& route_config : config.routes) {
    CompiledRoute compiled;
    compiled.config_ref = &route_config;

    compiled.watch_globs.reserve(route_config.watch.size());
    for (const auto& pattern : route_config.watch) {
      compiled.watch_globs.emplace_back(pattern);
    }

    compiled.ignore_globs.reserve(route_config.ignore.size());
    for (const auto& pattern : route_config.ignore) {
      compiled.ignore_globs.emplace_back(pattern);
    }

    compiled_routes_.push_back(std::move(compiled));
  }
}

void Router::route_events(const std::vector<event::Event>& debounced_events) {
  if (!dispatch_bc_) return;

  for (const auto& route : compiled_routes_) {
    std::vector<event::Event> matched_events;
    matched_events.reserve(debounced_events.size());

    for (const auto& event : debounced_events) {
      auto event_bits = static_cast<std::underlying_type_t<event::EventType>>(event.type);
      auto route_bits =
          static_cast<std::underlying_type_t<event::EventType>>(route.config_ref->events);

      if ((event_bits & route_bits) == 0) continue;

      std::string     path_str;
      std::error_code ec;
      auto            abs_path = std::filesystem::absolute(event.path, ec);
      auto            abs_root = std::filesystem::absolute(rootdir_, ec);
      auto            rel      = std::filesystem::relative(abs_path, abs_root, ec);
      if (!ec && !rel.empty())
        path_str = rel.string();
      else
        path_str = event.path.string();

      bool ignored = false;
      for (const auto& ignore_glob : route.ignore_globs) {
        if (glob::matches(path_str, ignore_glob)) {
          ignored = true;
          break;
        }
      }

      if (ignored) continue;

      bool watched = false;
      for (const auto& watch_glob : route.watch_globs) {
        if (glob::matches(path_str, watch_glob)) {
          watched = true;
          break;
        }
      }

      if (watched || route.watch_globs.empty()) matched_events.push_back(event);
    }

    if (!matched_events.empty()) dispatch_bc_(*route.config_ref, matched_events);
  }
}

}  // namespace voy::router
