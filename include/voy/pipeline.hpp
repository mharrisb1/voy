/*
 * markings:managed
 *
 * File: pipeline.hpp
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

#include <string>
#include <unordered_map>
#include <vector>

namespace voy::pipeline {

class Pipeline {
 public:
  [[nodiscard]] static std::unordered_map<std::string, std::string> prepare_environment(
      const config::RouteConfig& route, const std::vector<event::Event>& matched_events);
};

}  // namespace voy::pipeline
