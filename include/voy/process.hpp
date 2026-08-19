#pragma once

#include <voy/config.hpp>

#include <expected>
#include <string>
#include <unordered_map>

#include <sys/types.h>

namespace voy::process {

struct ProcessPipes {
  int stdout_fd{-1};
  int stderr_fd{-1};
};

class ProcessSupervisor {
 public:
  ProcessSupervisor() = default;
  ~ProcessSupervisor();

  std::expected<ProcessPipes, std::string> spawn(const config::ActionConfig&                         action,
                                                 const std::unordered_map<std::string, std::string>& env_map);

  void               kill_all();
  void               reap_zombies();
  [[nodiscard]] bool is_running() const { return pgid_ != -1; }

 private:
  pid_t pgid_{-1};
};

}  // namespace voy::process
