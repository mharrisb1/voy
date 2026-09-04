/*
 * markings:managed
 *
 * File: algorithms.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#pragma once
#include <algorithm>
#include <cctype>
#include <string>

namespace voy::cli::utils::algorithms {

std::string to_upper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
  return s;
}

}  // namespace voy::cli::utils::algorithms
