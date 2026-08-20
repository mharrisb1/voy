/*
 * markings:managed
 *
 * File: reactor.hpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#pragma once

#include <voy/event.hpp>

#include <expected>
#include <functional>
#include <unordered_map>

namespace voy::reactor {

class Reactor {
 public:
  using EventCallback  = std::function<void(const event::Event&)>;
  using SignalCallback = std::function<void()>;
  using FdCallback     = std::function<void(int)>;

  [[nodiscard]] static std::expected<Reactor, std::string> create();

  ~Reactor();
  Reactor(const Reactor&)            = delete;
  Reactor& operator=(const Reactor&) = delete;

  Reactor(Reactor&& other) noexcept;
  Reactor& operator=(Reactor&& other) noexcept;

  std::expected<int, std::string>  add_watch(const std::string& dir, event::EventType mask);
  std::expected<void, std::string> add_fd(int fd, FdCallback on_ready);
  void                             remove_fd(int fd);

  void run(EventCallback on_event, SignalCallback on_sigchld = nullptr);
  void stop();

 private:
  Reactor(int inotify_fd, int epoll_fd, int signal_fd);

  void handle_inotify_events(EventCallback& on_event);

  int                                  inotify_fd_{-1};
  int                                  epoll_fd_{-1};
  int                                  signal_fd_{-1};
  bool                                 running_{false};
  std::unordered_map<int, std::string> wd_to_path_;
  std::unordered_map<int, FdCallback>  custom_fds_;
};

}  // namespace voy::reactor
