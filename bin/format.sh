#!/usr/bin/env bash
set -e

find include src test -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) -exec clang-format -style=file -i {} +
