#pragma once

#include <voy/event.hpp>
#include <voy/glob.hpp>
#include <voy/reactor.hpp>

#include <filesystem>
#include <string_view>
#include <vector>

namespace voy::watch_tree {

class WatchTree {
 public:
  explicit WatchTree(reactor::Reactor& reactor);
  void add_ignore_rule(std::string_view glob_pattern);
  void watch_recursively(const std::filesystem::path& root_dir, event::EventType mask);
  [[nodiscard]] bool is_ignored(const std::filesystem::path& path) const;

 private:
  reactor::Reactor&              reactor_;
  std::vector<glob::GlobMatcher> ignore_rules_;
};

}  // namespace voy::watch_tree
