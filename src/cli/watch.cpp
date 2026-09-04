/*
 * markings:managed
 *
 * File: watch.cpp
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
#include <utility>

#include "config.cpp"
#include "parser.cpp"

using namespace voy::cli::parser;

namespace voy::cli::watch {

inline Command get_command() {
  return Command("watch").about("Start the event loop in the foreground").arg(Arg("debounce"));
}

inline int run(const parser::ArgMatches& root_matches, const parser::ArgMatches& watch_matches) {
  std::string config_path = watch_matches.get_one("config").value_or(
      root_matches.get_one("config").value_or(".voy.json"));

  std::string format_str =
      watch_matches.get_one("format").value_or(root_matches.get_one("format").value_or("json"));

  std::string rootdir =
      watch_matches.get_one("rootdir").value_or(root_matches.get_one("rootdir").value_or("."));

  bool no_vcs_ignore =
      watch_matches.get_flag("no-vcs-ignore") || root_matches.get_flag("no-vcs-ignore");

  bool no_project_ignore =
      watch_matches.get_flag("no-project-ignore") || root_matches.get_flag("no-project-ignore");

  auto format = voy::cli::config::Format::from_string(format_str);
  if (!format) {
    std::cerr << "[voy] Config Error: " << format.error() << "\n";
    return 1;
  }

  auto config = voy::cli::config::parse_config_file(config_path, format.value());
  if (!config) {
    std::cerr << "[voy] Config Error: " << config.error() << "\n";
    return 1;
  }

  if (!rootdir.empty()) config->rootdir = std::move(rootdir);
  if (no_vcs_ignore) config->no_vcs_ignore = true;
  if (no_project_ignore) config->no_project_ignore = true;

  auto on_stdout = [](std::string_view chunk) {
    std::cout << chunk;
    std::cout.flush();
  };

  auto on_stderr = [](std::string_view chunk) {
    std::cerr << chunk;
    std::cerr.flush();
  };

  auto engine_res = voy::engine::Engine::create(*config, on_stdout, on_stderr);
  if (!engine_res) {
    std::cerr << "[voy] Failed to initialize engine: " << engine_res.error() << "\n";
    return 1;
  }

  std::cout << "[voy] Watching for file changes (Config: " << config_path << ")...\n";
  engine_res->run();

  return 0;
}

}  // namespace voy::cli::watch
