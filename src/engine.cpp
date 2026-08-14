#include <voy/config.hpp>
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

namespace voy {

ActionBuilder Action::builder() {
  return ActionBuilder();
}

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

std::expected<Engine, std::string> EngineBuilder::build() {
  return Engine::create(config_);
}

std::expected<Engine, std::string> Engine::create(const config::VoyConfig& config) {
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
                std::move(*debouncer), std::move(router), std::move(supervisor));
}

void Engine::run() {
  router_.set_dispatch_callback(
      [this](const config::RouteConfig& route, const std::vector<event::Event>& events) {
        if (route.callback) route.callback(events);
        if (!route.action.command.empty()) {
          auto env = pipeline::Pipeline::prepare_environment(route, events);
          supervisor_.spawn(route.action, env);
        }
      });

  debouncer_.set_flush_callback(
      [this](std::vector<event::Event> events) { router_.route_events(events); });

  auto on_file_event = [this](const event::Event& event) {
    if ((event.raw_inotify_mask & IN_CREATE) && (event.raw_inotify_mask & IN_ISDIR)) {
      watch_tree.watch_recursively(event.path, event::EventType::All);
    }

    debouncer_.on_raw_event(event);
  };

  auto on_sigchld = [this]() { supervisor_.reap_zombies(); };

  for (const auto& route : config_.routes) {
    for (const auto& watch_dir : route.watch) {
      watch_tree.watch_recursively(watch_dir, event::EventType::All);
    }
  }

  reactor_->run(on_file_event, on_sigchld);
}

void Engine::stop() {
  if (reactor_) reactor_->stop();
}

}  // namespace voy
