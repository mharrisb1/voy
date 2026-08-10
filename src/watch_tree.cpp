#include <voy/event.hpp>
#include <voy/reactor.hpp>
#include <voy/watch_tree.hpp>

#include <filesystem>
#include <print>
#include <string_view>

namespace voy::watch_tree {
namespace fs = std::filesystem;

WatchTree::WatchTree(reactor::Reactor& reactor) : reactor_(reactor) {}

void WatchTree::add_ignore_rule(std::string_view glob_pattern) {
  ignore_rules_.emplace_back(glob_pattern);
}

bool WatchTree::is_ignored(const fs::path& path) const {
  std::string path_str = path.string();
  for (const auto& rule : ignore_rules_) {
    if (rule.matches(path_str)) return true;
  }
  return false;
}

void WatchTree::watch_recursively(const fs::path& root_dir, event::EventType mask) {
  std::error_code ec;

  if (!is_ignored(root_dir)) {
    auto res = reactor_.add_watch(root_dir.string(), mask);
    if (!res) {
      std::println(stderr, "[voy] Warning: Failed to watch directory {}: {}", root_dir.string(),
                   res.error());
    }
  } else {
    return;
  }

  auto it =
      fs::recursive_directory_iterator(root_dir, fs::directory_options::skip_permission_denied, ec);

  if (ec) {
    std::println(stderr, "[voy] Warning: Failed to open directory {}: {}", root_dir.string(),
                 ec.message());
    return;
  }

  for (auto end = fs::recursive_directory_iterator(); it != end; it.increment(ec)) {
    if (ec) {
      ec.clear();
      continue;
    }
    const auto& entry = *it;

    if (entry.is_directory(ec)) {
      if (is_ignored(entry.path())) {
        it.disable_recursion_pending();
      } else {
        auto res = reactor_.add_watch(entry.path().string(), mask);
        if (!res) {
          std::println(stderr, "[voy] Warning: Failed to watch directory {}: {}",
                       entry.path().string(), res.error());
        }
      }
    }
  }
}

}  // namespace voy::watch_tree
