/*
 * markings:managed
 *
 * File: router.hpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#pragma once

#include <voy/config.hpp>
#include <voy/event.hpp>

#include <functional>
#include <vector>

namespace voy::router {

class Router {
 public:
  using RouteMatchedCallback =
      std::function<void(const config::RouteConfig&, const std::vector<event::Event>&)>;

  explicit Router(const config::VoyConfig& config);

  void set_dispatch_callback(RouteMatchedCallback cb) { dispatch_bc_ = std::move(cb); }
  void route_events(const std::vector<event::Event>& debounced_events);

 private:
  std::vector<config::RouteConfig> routes_;
  RouteMatchedCallback             dispatch_bc_;
  std::string                      rootdir_;
};

}  // namespace voy::router
