/*
 * markings:managed
 *
 * File: main.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#include <voy/engine.hpp>

#include <iostream>
#include <string>
#include <string_view>

#include <getopt.h>

#include "config.hpp"

void print_help() {
  std::cout << "Usage: voy [options] <command>\n\n"
            << "Commands:\n"
            << "  watch                 Start the event loop in the foreground\n\n"
            << "Options:\n"
            << "  -c, --config <file>   Path to the JSON config file (default: .voy.json)\n"
            << "  -h, --help            Print this help and exit\n";
}

int main(int argc, char** argv) {
  std::string config_path = ".voy.json";

  static struct option long_options[] = {{"config", required_argument, nullptr, 'c'},
                                         {"help", no_argument, nullptr, 'h'},
                                         {nullptr, 0, nullptr, 0}};

  int opt;
  int option_index = 0;

  while ((opt = getopt_long(argc, argv, "c:h", long_options, &option_index)) != -1) {
    switch (opt) {
      case 'c': config_path = optarg; break;
      case 'h': print_help(); return 0;
      default: print_help(); return 1;
    }
  }

  if (optind >= argc) {
    std::cerr << "[voy] Error: No command specified.\n\n";
    print_help();
    return 1;
  }

  std::string command = argv[optind];

  if (command == "watch") {
    auto config_res = voy::cli::parse_config_file(config_path);
    if (!config_res) {
      std::cerr << "[voy] Config Error: " << config_res.error() << "\n";
      return 1;
    }

    auto on_stdout = [](std::string_view chunk) {
      std::cout << chunk;
      std::cout.flush();
    };

    auto on_stderr = [](std::string_view chunk) {
      std::cerr << "\033[31m" << chunk << "\033[0m";
      std::cerr.flush();
    };

    auto engine_res = voy::engine::Engine::create(*config_res, on_stdout, on_stderr);
    if (!engine_res) {
      std::cerr << "[voy] Failed to initialize engine: " << engine_res.error() << "\n";
      return 1;
    }

    std::cout << "[voy] Watching for file changes (Config: " << config_path << ")...\n";
    engine_res->run();

  } else {
    std::cerr << "[voy] Error: Unknown command '" << command << "'\n\n";
    print_help();
    return 1;
  }

  return 0;
}
