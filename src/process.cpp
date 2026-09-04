/*
 * markings:managed
 *
 * File: process.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#include <voy/process.hpp>

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <numeric>
#include <print>
#include <thread>

#include <fcntl.h>
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
      // TODO
    }

    kill(pgid_, SIGCONT);

    int      status;
    uint32_t elapsed_ms = 0;
    bool     exited     = false;

    while (elapsed_ms < grace_period_ms_) {
      pid_t res = waitpid(pgid_, &status, WNOHANG);
      if (res == pgid_ || res == -1) {
        exited = true;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      elapsed_ms += 10;
    }
    kill(-pgid_, SIGKILL);
    if (!exited) waitpid(pgid_, &status, 0);
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

std::expected<ProcessPipes, std::string> ProcessSupervisor::spawn(
    const config::ActionConfig&                         action,
    const std::unordered_map<std::string, std::string>& env_map) {
  if (is_running()) kill_all();

  grace_period_ms_ = action.grace_period_ms.value_or(10000);

  int out_pipe[2];
  int err_pipe[2];

  if (pipe2(out_pipe, O_NONBLOCK | O_CLOEXEC) == -1 ||
      pipe2(err_pipe, O_NONBLOCK | O_CLOEXEC) == -1) {
    return std::unexpected(std::string("[voy] Failed to create pipes: ") + strerror(errno));
  }

  pid_t pid = fork();

  if (pid < 0) {
    return std::unexpected(std::string("[voy] Failed to fork new process: ") + strerror(errno));
  }

  if (pid == 0) {
    setpgid(0, 0);

    dup2(out_pipe[1], STDOUT_FILENO);
    dup2(err_pipe[1], STDERR_FILENO);

    close(out_pipe[0]);
    close(out_pipe[1]);
    close(err_pipe[0]);
    close(err_pipe[1]);

    if (chdir(action.workdir.c_str()) == -1) {
      std::println(stderr, "[voy] Failed to chdir to {}", action.workdir);
      _exit(1);
    }

    for (const auto& [key, value] : env_map) {
      setenv(key.c_str(), value.c_str(), 1);
    }

    const char* shell = std::getenv("SHELL");
    if (!shell || std::strlen(shell) == 0) shell = "/bin/sh";

    auto concat  = [](const std::string& a, const std::string& b) { return a + " " + b; };
    auto command = std::accumulate(std::next(action.command.begin()), action.command.end(),
                                   action.command[0], concat);

    execl(shell, shell, "-c", command.c_str(), nullptr);

    std::println(stderr, "[voy] Failed to exec {}: {}", shell, strerror(errno));
    _exit(1);
  } else {
    pgid_ = pid;
    close(out_pipe[1]);
    close(err_pipe[1]);

    return ProcessPipes{out_pipe[0], err_pipe[0]};
  }
}

}  // namespace voy::process
