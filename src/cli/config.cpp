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
#include <glaze/toml.hpp>
#include <glaze/yaml.hpp>
#include <glaze/yaml/read.hpp>

template <>
struct glz::meta<voy::config::RouteConfig> {
  using T = voy::config::RouteConfig;

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
                  glz::custom<read_events, write_events>, "action", &T::action);
};

namespace voy::cli {

std::expected<voy::config::VoyConfig, std::string> parse_config_file(const std::string& path,
                                                                     ConfigFormat       format) {
  voy::config::VoyConfig config;
  std::string            buffer;

  glz::error_ctx ec;

  switch (format) {
    case ConfigFormat::Json: ec = glz::read_file_json(config, path, buffer); break;
    case ConfigFormat::Toml: ec = glz::read_file_toml(config, path, buffer); break;
    case ConfigFormat::Yaml: ec = glz::read_file_yaml(config, path, buffer); break;
  }

  if (ec)
    return std::unexpected(static_cast<std::string>(config_format_to_string(format)) +
                           " error: " + glz::format_error(ec, buffer));
  return config;
}

}  // namespace voy::cli
