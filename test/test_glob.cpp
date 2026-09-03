/*
 * markings:managed
 *
 * File: test_glob.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#include <voy/glob.hpp>

#include <tuple>
#include <vector>

#include <doctest/doctest.h>

TEST_CASE("voy::glob::matches") {
  const std::vector<std::tuple<std::string, std::string, bool>> TEST_CASES = {
      // 1. Literal and simple wildcard matches
      {"file.txt", "file.txt", true},
      {"src/file.txt", "file.txt", true},
      {"file.jpg", "*.txt", false},
      {"file.txt", "*.txt", true},
      {"src/file.txt", "*.txt", true},

      // 2. Anchored patterns (leading or internal slash)
      {"file.txt", "/file.txt", true},
      {"src/file.txt", "/file.txt", false},
      {"src/file.txt", "src/file.txt", true},
      {"sub/src/file.txt", "src/file.txt", false},

      // 3. Directory matching (trailing slash)
      {"build", "build/", true},
      {"build/debug.log", "build/", true},
      {"src/build", "build/", true},
      {"src/build/debug.log", "build/", true},

      // 4. Single-character wildcard
      {"file1.txt", "file?.txt", true},
      {"file12.txt", "file?.txt", false},

      // 5. Character sets and ranges
      {"file-a.txt", "file-[a-z].txt", true},
      {"file-A.txt", "file-[a-z].txt", false},

      // 6. Globstar (**) recursive wildcards
      {"src/index.ts", "src/**/*.ts", true},
      {"src/utils/helpers.ts", "src/**/*.ts", true},
      {"src/components/button/index.ts", "src/**/*.ts", true},
      {"index.ts", "src/**/*.ts", false},
      {"logs/2026/09/error.log", "logs/**/error.log", true},

      // 7. Braces treat as literal (Not supported in gitignore)
      {"file.js", "*.{js,ts}", false},
      {"file.{js,ts}", "*.{js,ts}", true},

      // 8. .voy/test.json patterns
      {"src/main.cpp", "src/**/*.cpp", true},
      {"src/utils/math.cpp", "src/**/*.cpp", true},
      {"src/utils/math/math.cpp", "src/**/*.cpp", true},
      {"include/main.hpp", "include/**/*.hpp", true},
      {"include/utils/math.hpp", "include/**/*.hpp", true},
      {"include/main.h", "include/**/*.h", true},
      {"test/test_glob.cpp", "test/**/*.cpp", true},
      {"src/main.c", "src/**/*.cpp", false},
      {"include/main.cpp", "include/**/*.hpp", false}};

  for (const auto& [path, pattern, expected] : TEST_CASES) {
    INFO("path=", path, "\tpattern=", pattern, "\texpected=", expected);
    CHECK(voy::glob::matches(path, pattern) == expected);
  }
}
