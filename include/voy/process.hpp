#pragma once

#include <voy/config.hpp>

#include <unordered_map>

#include <sys/types.h>

namespace voy::process {

class ProcessSupervisor {
 public:
  ProcessSupervisor() = default;
  ~ProcessSupervisor();

  void spawn(const config::ActionConfig&                         action,
             const std::unordered_map<std::string, std::string>& env_map);

  void               kill_all();
  void               reap_zombies();
  [[nodiscard]] bool is_running() const { return pgid_ != -1; }

 private:
  pid_t pgid_{-1};
};

}  // namespace voy::process
