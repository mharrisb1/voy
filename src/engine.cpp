/*
 * markings:managed
 *
 * File: engine.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#include <voy/config.hpp>
#include <voy/glob.hpp>
#include <voy/debounce.hpp>
#include <voy/engine.hpp>
#include <voy/event.hpp>
#include <voy/pipeline.hpp>
#include <voy/process.hpp>
#include <voy/reactor.hpp>
#include <voy/router.hpp>
#include <voy/watch_tree.hpp>

#include <chrono>
#include <memory>
#include <optional>
#include <vector>

#include <sys/inotify.h>
#include <unistd.h>

namespace voy::engine {

ActionBuilder Action::builder() {
  return ActionBuilder();
}

ActionBuilder::ActionBuilder() = default;

ActionBuilder& ActionBuilder::with_command(std::string command) {
  config_.command = std::move(command);
  return *this;
}

ActionBuilder& ActionBuilder::with_workdir(std::string workdir) {
  config_.workdir = std::move(workdir);
  return *this;
}

ActionBuilder& ActionBuilder::set_env(std::string key, std::string value) {
  config_.env[key] = value;
  return *this;
}

config::ActionConfig ActionBuilder::build() {
  return std::move(config_);
}

RouteBuilder Route::builder(std::optional<std::string> name) {
  return RouteBuilder(std::move(name));
}

RouteBuilder::RouteBuilder(std::optional<std::string> name) {
  if (name) { config_.name = std::move(*name); }
}

RouteBuilder& RouteBuilder::watch(std::string glob) {
  config_.watch.push_back(std::move(glob));
  return *this;
}

RouteBuilder& RouteBuilder::ignore(std::string glob) {
  config_.watch.push_back(std::move(glob));
  return *this;
}

RouteBuilder& RouteBuilder::on_events(event::EventType events) {
  config_.events = events;
  return *this;
}

RouteBuilder& RouteBuilder::with_action(const config::ActionConfig& action) {
  config_.action = std::move(action);
  return *this;
}

RouteBuilder& RouteBuilder::with_callback(config::NativeCallback cb) {
  config_.callback = std::move(cb);
  return *this;
}

config::RouteConfig RouteBuilder::build() {
  return std::move(config_);
}

EngineBuilder Engine::builder() {
  return EngineBuilder();
}

EngineBuilder& EngineBuilder::with_debounce_window(std::chrono::milliseconds window) {
  config_.debounce_ms = static_cast<uint32_t>(window.count());
  return *this;
}

EngineBuilder& EngineBuilder::add_route(const config::RouteConfig& route) {
  config_.routes.push_back(std::move(route));
  return *this;
}

EngineBuilder& EngineBuilder::on_stdout(std::function<void(std::string_view)> cb) {
  on_stdout_ = std::move(cb);
  return *this;
}

EngineBuilder& EngineBuilder::on_stderr(std::function<void(std::string_view)> cb) {
  on_stderr_ = std::move(cb);
  return *this;
}

std::expected<Engine, std::string> EngineBuilder::build() {
  return Engine::create(config_, std::move(on_stdout_), std::move(on_stderr_));
}

std::expected<Engine, std::string> Engine::create(const config::VoyConfig&              config,
                                                  std::function<void(std::string_view)> on_stdout,
                                                  std::function<void(std::string_view)> on_stderr) {
  auto reactor_res = reactor::Reactor::create();
  if (!reactor_res) return std::unexpected(reactor_res.error());

  auto                  reactor_ptr = std::make_unique<reactor::Reactor>(std::move(*reactor_res));
  watch_tree::WatchTree watch_tree(*reactor_ptr);

  for (const auto& route : config.routes) {
    for (const auto& ignore_glob : route.ignore) {
      watch_tree.add_ignore_rule(ignore_glob);
    }
  }

  auto debouncer = debounce::Debouncer::create(std::chrono::milliseconds(150));
  if (!debouncer) return std::unexpected(debouncer.error());

  router::Router             router(config);
  process::ProcessSupervisor supervisor;

  return Engine(std::move(config), std::move(reactor_ptr), std::move(watch_tree),
                std::move(*debouncer), std::move(router), std::move(supervisor),
                std::move(on_stdout), std::move(on_stderr));
}

Engine::Engine(const config::VoyConfig& config, std::unique_ptr<reactor::Reactor> reactor,
               watch_tree::WatchTree watch_tree, debounce::Debouncer debouncer,
               router::Router router, process::ProcessSupervisor supervisor,
               std::function<void(std::string_view)> on_stdout,
               std::function<void(std::string_view)> on_stderr)
    : config_(config),
      reactor_(std::move(reactor)),
      watch_tree_(std::move(watch_tree)),
      debouncer_(std::move(debouncer)),
      router_(std::move(router)),
      supervisor_(std::move(supervisor)),
      on_stdout_(std::move(on_stdout)),
      on_stderr_(std::move(on_stderr)) {}

void Engine::run() {
  auto drain_pipe = [this](int fd, buffer::RingBuffer& ring, const auto& consumer_cb) {
    char buf[4096];
    while (true) {
      ssize_t bytes = read(fd, buf, sizeof(buf));
      if (bytes > 0) {
        std::string_view chunk(buf, static_cast<size_t>(bytes));
        ring.push(chunk);
        if (consumer_cb) consumer_cb(chunk);
      } else if (bytes == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
        reactor_->remove_fd(fd);
        close(fd);
        break;
      } else {
        reactor_->remove_fd(fd);
        close(fd);
        break;
      }
    }
  };

  router_.set_dispatch_callback([this, drain_pipe](const config::RouteConfig&       route,
                                                   const std::vector<event::Event>& events) {
    if (route.callback) route.callback(events);
    if (!route.action.command.empty()) {
      auto env = pipeline::Pipeline::prepare_environment(route, events);
      auto pipes_res = supervisor_.spawn(route.action, env);

      if (!pipes_res) {
        std::string err = pipes_res.error() + "\n";
        stderr_ring_.push(err);
        if (on_stderr_) on_stderr_(err);
        return;
      }

      auto pipes = *pipes_res;

      if (pipes.stdout_fd != -1) {
        auto res = reactor_->add_fd(pipes.stdout_fd, [this, drain_pipe](int fd) {
          drain_pipe(fd, stdout_ring_, on_stdout_);
        });
        if (!res) {
          std::string err = "[voy] Failed to monitor stdout: " + res.error() + "\n";
          stderr_ring_.push(err);
          if (on_stderr_) on_stderr_(err);
        }
      }
      if (pipes.stderr_fd != -1) {
        auto res = reactor_->add_fd(pipes.stderr_fd, [this, drain_pipe](int fd) {
          drain_pipe(fd, stderr_ring_, on_stderr_);
        });
        if (!res) {
          std::string err = "[voy] Failed to monitor stderr: " + res.error() + "\n";
          stderr_ring_.push(err);
          if (on_stderr_) on_stderr_(err);
        }
      }
    }
  });

  debouncer_.set_flush_callback(
      [this](std::vector<event::Event> events) { router_.route_events(events); });

  auto debouncer_res = reactor_->add_fd(debouncer_.timer_fd(), [this](int) { debouncer_.on_timer_expired(); });
  if (!debouncer_res) {
    std::string err = "[voy] Failed to monitor debouncer timer: " + debouncer_res.error() + "\n";
    stderr_ring_.push(err);
    if (on_stderr_) on_stderr_(err);
  }

  auto on_file_event = [this](const event::Event& event) {
    if ((event.raw_inotify_mask & IN_CREATE) && (event.raw_inotify_mask & IN_ISDIR)) {
      watch_tree_.watch_recursively(event.path, event::EventType::All);
    }

    debouncer_.on_raw_event(event);
  };

  auto on_sigchld = [this]() { supervisor_.reap_zombies(); };

  for (const auto& route : config_.routes) {
    for (const auto& watch_glob : route.watch) {
      std::string base_dir = glob::GlobMatcher::extract_base_dir(watch_glob);
      watch_tree_.watch_recursively(base_dir, event::EventType::All);
    }
  }

  reactor_->run(on_file_event, on_sigchld);
}

void Engine::stop() {
  if (reactor_) reactor_->stop();
}

}  // namespace voy::engine
