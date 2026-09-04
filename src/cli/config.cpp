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

#pragma once
#include <voy/config.hpp>
#include <voy/event.hpp>

#include <expected>
#include <optional>
#include <string_view>

#include <glaze/glaze.hpp>
#include <glaze/yaml.hpp>

#include "utils/algorithms.cpp"

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

namespace voy::cli::config {

struct Format {
  enum Value { Json, Yaml };
  Value value;
  constexpr Format(Value v) : value(v) {}
  constexpr                  operator Value() const { return value; }
  constexpr std::string_view to_string() const {
    switch (value) {
      case Json: return "JSON";
      case Yaml: return "YAML";
      default: return "Unknown";
    }
  }
  static constexpr std::expected<Format, std::string> from_string(std::string_view s) {
    auto str = voy::cli::utils::algorithms::to_upper(static_cast<std::string>(s));
    if (str == "JSON") return Format::Json;
    if (str == "YAML") return Format::Yaml;
    return std::unexpected("invalid format " + static_cast<std::string>(s));
  }
};

std::expected<voy::config::VoyConfig, std::string> parse_config_file(const std::string& path,
                                                                     Format             format) {
  voy::config::VoyConfig config;
  std::string            buffer;

  glz::error_ctx ec;

  switch (format) {
    case Format::Json: ec = glz::read_file_json(config, path, buffer); break;
    case Format::Yaml: ec = glz::read_file_yaml(config, path, buffer); break;
  }

  if (ec) {
    auto format_str = static_cast<std::string>(format.to_string());
    return std::unexpected(format_str + " error: " + glz::format_error(ec, buffer));
  }
  return config;
}

}  // namespace voy::cli::config
