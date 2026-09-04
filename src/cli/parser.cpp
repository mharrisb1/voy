/*
 * markings:managed
 *
 * File: parser.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#pragma once
#include <cstdlib>
#include <expected>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace voy::cli::parser {

enum class ArgAction { StoreValue, SetTrue };

class ArgMatches {
 public:
  bool has(const std::string& name) const {
    return m_values.contains(name) || m_flags.contains(name);
  }

  std::optional<std::string> get_one(const std::string& name) const {
    if (auto it = m_values.find(name); it != m_values.end()) return it->second;
    return std::nullopt;
  }

  bool get_flag(const std::string& name) const {
    if (auto it = m_flags.find(name); it != m_flags.end()) return it->second;
    return false;
  }

  const std::string& subcommand_name() const { return m_subcommand_name; }
  const ArgMatches*  subcommand_matches() const { return m_subcommand_matches.get(); };

  void set_value(const std::string& name, std::string value) { m_values[name] = std::move(value); }
  void set_flag(const std::string& name, bool value) { m_flags[name] = value; }

  void set_subcommand(std::string name, std::unique_ptr<ArgMatches> matches) {
    m_subcommand_name    = std::move(name);
    m_subcommand_matches = std::move(matches);
  }

 private:
  std::map<std::string, std::string> m_values;
  std::map<std::string, bool>        m_flags;
  std::string                        m_subcommand_name;
  std::unique_ptr<ArgMatches>        m_subcommand_matches;
};

class Arg {
 public:
  explicit Arg(std::string name) : m_name(std::move(name)) {}

  Arg& short_name(char c) {
    m_short_name = c;
    return *this;
  }

  Arg& long_name(std::string name) {
    m_long_name = std::move(name);
    return *this;
  }

  Arg& help(std::string text) {
    m_help = std::move(text);
    return *this;
  }

  Arg& default_value(std::string val) {
    m_default_value = std::move(val);
    return *this;
  }

  Arg& action(ArgAction act) {
    m_action = act;
    return *this;
  }

  Arg& required(bool req = true) {
    m_required = req;
    return *this;
  }

  Arg& global(bool glob = true) {
    m_global = glob;
    return *this;
  }

  Arg& remove_default() {
    m_default_value = std::nullopt;
    return *this;
  }

  const std::string&                name() const { return m_name; }
  std::optional<char>               get_short() const { return m_short_name; }
  const std::optional<std::string>& get_long() const { return m_long_name; }
  const std::optional<std::string>& get_help() const { return m_help; }
  const std::optional<std::string>& get_default() const { return m_default_value; }
  ArgAction                         get_action() const { return m_action; }
  bool                              is_required() const { return m_required; }
  bool                              is_global() const { return m_global; }

 private:
  std::string                m_name;
  std::optional<char>        m_short_name;
  std::optional<std::string> m_long_name;
  std::optional<std::string> m_help;
  std::optional<std::string> m_default_value;
  ArgAction                  m_action   = ArgAction::StoreValue;
  bool                       m_required = false;
  bool                       m_global   = false;
};

class Command {
 public:
  explicit Command(std::string name) : m_name(std::move(name)) {}

  Command& about(std::string text) {
    m_about = std::move(text);
    return *this;
  }

  Command& arg(Arg arg) {
    m_args.push_back(std::move(arg));
    return *this;
  }

  Command& subcommand(Command subcmd) {
    m_subcommands.push_back(std::move(subcmd));
    return *this;
  }

  std::expected<ArgMatches, std::string> parse(int argc, const char* const* argv) const {
    int current_index = 1;
    return parse_internal(argc, argv, current_index);
  }

  void print_help(std::ostream& os = std::cout) const {
    os << "Usage: " << m_name;
    if (!m_args.empty()) os << " [OPTIONS]";
    if (!m_subcommands.empty()) os << " <COMMAND>";
    os << "\n";

    if (m_about) os << "\n" << *m_about << "\n";

    if (!m_subcommands.empty()) {
      os << "\nCommands:\n";
      for (const auto& subcmd : m_subcommands) {
        os << "  " << subcmd.name() << "\t\t";
        if (subcmd.get_about()) os << *subcmd.get_about();
        os << "\n";
      }
    }

    os << "\nOptions:\n";
    os << "  -h, --help\t\t\tPrint this help and exit\n";

    for (const auto& a : m_args) {
      os << "  ";
      bool has_short = false;
      if (a.get_short()) {
        os << "-" << *a.get_short();
        has_short = true;
      }
      if (a.get_long()) {
        if (has_short) os << ", ";
        os << "--" << *a.get_long();
      }

      if (a.get_action() == ArgAction::StoreValue) { os << " <val>"; }

      os << "\t\t";
      if (a.get_help()) os << *a.get_help() << " ";
      if (a.get_default()) os << "(default: " << *a.get_default() << ")";
      if (a.is_required()) os << "[required]";
      os << "\n";
    }
  }

  const std::string&                name() const { return m_name; }
  const std::optional<std::string>& get_about() const { return m_about; }
  const std::vector<Arg>&           get_args() const { return m_args; }
  const std::vector<Command>&       get_subcommands() const { return m_subcommands; }

 private:
  std::string                m_name;
  std::optional<std::string> m_about;
  std::vector<Arg>           m_args;
  std::vector<Command>       m_subcommands;

  std::expected<ArgMatches, std::string> parse_internal(int argc, const char* const* argv,
                                                        int& idx) const {
    ArgMatches matches;

    while (idx < argc) {
      std::string token = argv[idx];

      if (token == "-h" || token == "--help") {
        print_help();
        std::exit(0);
      }

      if (!token.starts_with("-")) {
        bool found_subcommand = false;
        for (auto subcmd : m_subcommands) {
          if (subcmd.name() == token) {
            idx++;

            for (const auto& a : m_args) {
              if (a.is_global()) {
                Arg inherited = a;
                inherited.remove_default();
                subcmd.arg(inherited);
              }
            }
            auto sub_matches = subcmd.parse_internal(argc, argv, idx);
            if (!sub_matches) return sub_matches;

            matches.set_subcommand(token, std::make_unique<ArgMatches>(std::move(*sub_matches)));
            found_subcommand = true;
            break;
          }
        }

        if (found_subcommand) {
          break;
        } else {
          return std::unexpected("Unknown command or positional argument: " + token);
        }
      }

      const Arg* matched_arg = nullptr;

      if (token.starts_with("--")) {
        std::string long_name = token.substr(2);
        for (const auto& a : m_args) {
          if (a.get_long() == long_name) {
            matched_arg = &a;
            break;
          }
        }
      } else if (token.starts_with("-") && token.size() == 2) {
        char short_name = token[1];
        for (const auto& a : m_args) {
          if (a.get_short() == short_name) {
            matched_arg = &a;
            break;
          }
        }
      }

      if (!matched_arg) { return std::unexpected("Unknown argument: " + token); }

      if (matched_arg->get_action() == ArgAction::SetTrue) {
        matches.set_flag(matched_arg->name(), true);
        idx++;
      } else if (matched_arg->get_action() == ArgAction::StoreValue) {
        if (idx + 1 >= argc) {
          return std::unexpected("Argument '" + token + "' requires a value");
        }
        matches.set_value(matched_arg->name(), argv[idx + 1]);
        idx += 2;
      }
    }

    for (const auto& a : m_args) {
      if (!matches.has(a.name())) {
        if (a.is_required()) {
          return std::unexpected("Missing required argument: " + a.name());
        } else if (a.get_default().has_value() && a.get_action() == ArgAction::StoreValue) {
          matches.set_value(a.name(), a.get_default().value());
        }
      }
    }

    return matches;
  }
};

}  // namespace voy::cli::parser
