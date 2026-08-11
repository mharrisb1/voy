#include <voy/config.hpp>
#include <voy/event.hpp>
#include <voy/router.hpp>

#include <type_traits>
#include <utility>

namespace voy::router {

Router::Router(const config::VoyConfig& config) {
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

      std::string path_str = event.path.string();

      bool ignored = false;
      for (const auto& ignore_glob : route.ignore_globs) {
        if (ignore_glob.matches(path_str)) {
          ignored = true;
          break;
        }
      }

      if (ignored) continue;

      bool watched = false;
      for (const auto& watch_glob : route.watch_globs) {
        if (watch_glob.matches(path_str)) {
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
