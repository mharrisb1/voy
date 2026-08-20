#!/usr/bin/env bash
set -e

cmake -B build
cmake --build build --target voy
sudo cp ./build/voy /usr/local/bin
