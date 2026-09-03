/*
 * markings:managed
 *
 * File: config.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#include "config.hpp"

#include <voy/config.hpp>
#include <voy/event.hpp>

#include <expected>
#include <string>
#include <vector>

#include <glaze/glaze.hpp>

template <>
struct glz::meta<voy::config::RouteConfig> {
  using T = voy::config::RouteConfig;

  // Take the vector by value since from_strings expects a mutable reference
  static constexpr auto read_events = [](T& s, std::vector<std::string> strings,
                                         glz::context& ctx) {
    if (auto type = voy::event::from_strings(strings)) {
      s.events = *type;
    } else {
      ctx.error = glz::error_code::syntax_error;
    }
  };

  static constexpr auto write_events = [](const T& s) {
    return voy::event::to_composite_string(s.events);
  };

  static constexpr auto value =
      glz::object("name", &T::name, "watch", &T::watch, "ignore", &T::ignore, "events",
                  glz::custom<read_events, write_events>, "action", &T::action
                  // callback is intentionally omitted so it stays ignored
      );
};

namespace voy::cli {

std::expected<voy::config::VoyConfig, std::string> parse_config_file(const std::string& path) {
  voy::config::VoyConfig config;
  std::string            buffer;

  auto ec = glz::read_file_json(config, path, buffer);
  if (ec) return std::unexpected("JSON error: " + glz::format_error(ec, buffer));
  return config;
}

}  // namespace voy::cli
