/*
 * markings:managed
 *
 * File: config.hpp
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

#include <expected>
#include <string_view>

namespace voy::cli {

enum class ConfigFormat { Json, Toml, Yaml };

constexpr std::string_view config_format_to_string(ConfigFormat format) {
  switch (format) {
    case ConfigFormat::Json: return "JSON";
    case ConfigFormat::Toml: return "TOML";
    case ConfigFormat::Yaml: return "YAML";
    default: return "Unknown";
  }
}

std::expected<voy::config::VoyConfig, std::string> parse_config_file(
    const std::string& path, ConfigFormat format = ConfigFormat::Json);

}  // namespace voy::cli
