#include <voy/process.hpp>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <print>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace voy::process {

ProcessSupervisor::~ProcessSupervisor() {
  kill_all();
}

void ProcessSupervisor::kill_all() {
  if (is_running()) {
    if (kill(-pgid_, SIGTERM) == -1) {
      // TODO: do we need to handle?
    }

    kill(pgid_, SIGCONT);

    int status;
    waitpid(pgid_, &status, 0);

    pgid_ = -1;
  }
}

void ProcessSupervisor::reap_zombies() {
  int   status;
  pid_t pid;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    if (pid == pgid_) pgid_ = -1;
  }
}

void ProcessSupervisor::spawn(const config::ActionConfig&                         action,
                              const std::unordered_map<std::string, std::string>& env_map) {
  if (is_running()) kill_all();

  pid_t pid = fork();

  if (pid < 0) {
    std::println(stderr, "[voy] Failed to fork new process: {}", strerror(errno));
    return;
  }

  if (pid == 0) {
    setpgid(0, 0);

    if (chdir(action.workdir.c_str()) == -1) {
      std::println(stderr, "[voy] Failed to chdir to {}", action.workdir);
      _exit(1);
    }

    for (const auto& [key, value] : env_map) {
      setenv(key.c_str(), value.c_str(), 1);
    }

    const char* shell = std::getenv("SHELL");
    if (!shell || std::strlen(shell) == 0) shell = "/bin/sh";

    execl(shell, shell, "-c", action.command.c_str(), nullptr);

    std::println(stderr, "[voy] Failed to exec {}: {}", shell, strerror(errno));
    _exit(1);
  } else {
    pgid_ = pid;
  }
}

}  // namespace voy::process
