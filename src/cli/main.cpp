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

#include <iostream>
#include <ostream>
#include <string_view>

#include "parser.cpp"
#include "watch.cpp"

using namespace voy::cli::parser;

int main(int argc, char** argv) {
  auto cli = Command("voy")
                 .about("Naughty little file watcher")
                 .arg(Arg("config")
                          .short_name('c')
                          .long_name("config")
                          .default_value(".voy.json")
                          .help("Path to the config file")
                          .global(true))
                 .arg(Arg("format")
                          .short_name('f')
                          .long_name("format")
                          .default_value("json")
                          .help("Config format (json, toml, yaml)")
                          .global(true))
                 .arg(Arg("rootdir")
                          .short_name('r')
                          .long_name("rootdir")
                          .default_value(".")
                          .help("Root directory for watcher")
                          .global(true))
                 .arg(Arg("no-vcs-ignore")
                          .long_name("no-vcs-ignore")
                          .action(ArgAction::SetTrue)
                          .help("Don't load .gitignore")
                          .global(true))
                 .arg(Arg("no-project-ignore")
                          .long_name("no-project-ignore")
                          .action(ArgAction::SetTrue)
                          .help("Don't load .ignore")
                          .global(true))
                 .subcommand(voy::cli::watch::get_command());

  auto parse_res = cli.parse(argc, argv);
  if (!parse_res) {
    std::cerr << "[voy] Error: " << parse_res.error() << "\n\n";
    cli.print_help(std::cerr);
    return 1;
  }

  const auto&      matches = *parse_res;
  std::string_view subcmd  = matches.subcommand_name();

  if (subcmd == "watch") {
    return voy::cli::watch::run(matches, *matches.subcommand_matches());
  } else if (subcmd.empty()) {
    std::cerr << "[voy] Error: No command specified.\n\n";
    cli.print_help(std::cerr);
    return 1;
  }

  return 0;
}
