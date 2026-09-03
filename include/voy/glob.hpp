/*
 * markings:managed
 *
 * File: glob.hpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#pragma once

#include <string>

namespace voy::glob {
[[nodiscard]] bool matches(std::string path, std::string pattern);
}
