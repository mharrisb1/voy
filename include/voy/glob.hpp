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

#include <regex>
#include <string_view>

namespace voy::glob {

class GlobMatcher {
 public:
  explicit GlobMatcher(std::string_view pattern);
  [[nodiscard]] bool               matches(std::string_view path) const;
  [[nodiscard]] std::string_view   pattern() const noexcept { return pattern_; }
  [[nodiscard]] static std::string to_regex_string(std::string_view glob);
  [[nodiscard]] static std::string extract_base_dir(std::string_view glob);

 private:
  std::string pattern_;
  std::regex  regex_;
};

}  // namespace voy::glob
