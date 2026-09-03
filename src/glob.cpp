/*
 * markings:managed
 *
 * File: glob.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#include <voy/glob.hpp>

#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace voy::glob {

/** https://github.com/Robert-van-Engelen/FastGlobbing/blob/f08b52933fbe07fbcb8dd8295387dba7c0abb0dd/cpp/match.cpp#L166-L323 */
bool core_match(std::string_view path, std::string_view pattern) {
  size_t i = 0;
  size_t j = 0;
  size_t n = path.size();
  size_t m = pattern.size();

  size_t path_backup_primary      = std::string::npos;
  size_t pattern_backup_primary   = std::string::npos;
  size_t path_backup_secondary    = std::string::npos;
  size_t pattern_backup_secondary = std::string::npos;

  while (i < n) {
    if (j < m) {
      switch (pattern[j]) {
        case '*':
          // Match anything except . after /
          if (path[i] == '.' && (i == 0 || path[i - 1] == '/')) break;
          if (++j < m && pattern[j] == '*') {
            // Trailing ** match everything after
            if (++j >= m) return true;
            // If ** followed / match zero or more dirs
            if (pattern[j] != '/') return false;

            // New **-loop, discard *-loop
            path_backup_primary      = std::string::npos;
            pattern_backup_primary   = std::string::npos;
            path_backup_secondary    = i;
            pattern_backup_secondary = j;

            if (path[i] != '/') j++;
            continue;
          }
          // Trailing * matches everything except /
          path_backup_primary    = i;
          pattern_backup_primary = j;
          continue;

        case '?':
          // Match anything except . after /
          if (path[i] == '.' && (i == 0 || path[i - 1] == '/')) break;
          if (path[i] == '/') break;
          i++;
          j++;
          continue;

        case '[': {
          // Match anything except . after /
          if (path[i] == '.' && (i == 0 || path[i - 1] == '/')) break;

          // Match any character in [...] except /
          if (path[i] == '/') break;
          bool matched = false;
          bool reverse = j + 1 < m && (pattern[j + 1] == '^' || pattern[j + 1] == '!');
          // Inverted character class
          if (reverse) j++;
          // Match character class
          for (int lastchar = 256; ++j < m && pattern[j] != ']'; lastchar = pattern[j]) {
            if (lastchar < 256 && pattern[j] == '-' && j + 1 < m && pattern[j + 1] != ']'
                    ? path[i] <= pattern[++j] && path[i] >= lastchar
                    : path[i] == pattern[j]) {
              matched = true;
            }
          }
          if (matched == reverse) break;
          i++;
          if (j < m) j++;
          continue;
        }

        case '\\':
          // Literal match \-escaped character
          if (j + 1 < m) j++;
          // FALLTHROUGH

        default:
          // Match the current non-NUL character
          if (pattern[j] != path[i] && !(pattern[j] == '/' && path[i] == '/')) break;
          i++;
          j++;
          continue;
      }
    }
    if (pattern_backup_primary != std::string::npos && path[path_backup_primary] != '/') {
      // *-loop: backtrack to the last * but do not jump over /
      i = ++path_backup_primary;
      j = pattern_backup_primary;
      continue;
    }
    if (pattern_backup_secondary != std::string::npos) {
      // **-loop: backtrack to the last **
      i = ++path_backup_secondary;
      j = pattern_backup_secondary;
      continue;
    }
    return false;
  }
  // Ignore trailing stars
  while (j < m && pattern[j] == '*')
    j++;
  // At the end of text means success if nothing else is left to match
  return j >= m;
}

bool matches(std::string path, std::string pattern) {
  if (pattern.empty()) return false;

  std::string p = pattern;

  // Expand tilde
  if (p[0] == '~') {
    const auto home = std::getenv("HOME");
    if (home == nullptr) return false;
    if (p.size() >= 2 && p[1] == '/') {
      p = std::string(home) + p.substr(1);
    } else {
      auto        sep = strrchr(home, '/');
      std::string home_dir =
          sep == nullptr ? "" : std::string(home, static_cast<size_t>(sep - home) + 1);
      p = home_dir + p.substr(1);
    }
  }

  if (!p.empty() && p.back() == '/') p.pop_back();

  bool anchored = false;
  if (!p.empty() && p.front() == '/') {
    anchored = true;
    p.erase(0, 1);
  } else if (p.find('/') != std::string::npos) {
    anchored = true;
  }

  size_t pos = 0;
  while (pos <= path.size()) {
    size_t next_slash = path.find('/', pos);
    size_t end        = (next_slash == std::string::npos) ? path.size() : next_slash;

    std::string_view prefix(path.data(), end);

    if (anchored) {
      if (core_match(prefix, p)) return true;
    } else {
      size_t           last_slash = prefix.rfind('/');
      std::string_view basename =
          (last_slash == std::string::npos) ? prefix : prefix.substr(last_slash + 1);
      if (core_match(basename, p)) return true;
    }

    if (next_slash == std::string::npos) break;
    pos = next_slash + 1;
  }

  return false;
}
}  // namespace voy::glob
