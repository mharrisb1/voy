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

namespace voy::cli {

std::expected<voy::config::VoyConfig, std::string> parse_config_file(const std::string& path);

}
