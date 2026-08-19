#pragma once

#include <voy/config.hpp>
#include <voy/debounce.hpp>
#include <voy/event.hpp>
#include <voy/process.hpp>
#include <voy/reactor.hpp>
#include <voy/router.hpp>
#include <voy/watch_tree.hpp>

#include <chrono>
#include <expected>
#include <memory>
#include <optional>
#include <string>

namespace voy::engine {

class ActionBuilder;
class RouteBuilder;
class EngineBuilder;

class Action {
 public:
  static ActionBuilder builder();
};

class ActionBuilder {
 public:
  explicit ActionBuilder();

  ActionBuilder& with_command(std::string command);
  ActionBuilder& with_workdir(std::string);
  ActionBuilder& set_env(std::string key, std::string value);
  // TODO: add remaining fields

  config::ActionConfig build();

 private:
  config::ActionConfig config_;
};

class Route {
 public:
  static RouteBuilder builder(std::optional<std::string> name = std::nullopt);
};

class RouteBuilder {
 public:
  explicit RouteBuilder(std::optional<std::string> name);

  RouteBuilder& watch(std::string glob);
  RouteBuilder& ignore(std::string glob);
  RouteBuilder& on_events(event::EventType events);
  RouteBuilder& with_action(const config::ActionConfig& action);
  RouteBuilder& with_callback(config::NativeCallback cb);

  config::RouteConfig build();

 private:
  config::RouteConfig config_;
};

class Engine {
 public:
  static EngineBuilder builder();

  static std::expected<Engine, std::string> create(const config::VoyConfig& config);

  void run();
  void stop();

 private:
  Engine(const config::VoyConfig& config, std::unique_ptr<reactor::Reactor> reactor,
         watch_tree::WatchTree watch_tree, debounce::Debouncer debouncer, router::Router router,
         process::ProcessSupervisor supervisor);

  config::VoyConfig                 config_;
  std::unique_ptr<reactor::Reactor> reactor_;
  watch_tree::WatchTree             watch_tree;
  debounce::Debouncer               debouncer_;
  router::Router                    router_;
  process::ProcessSupervisor        supervisor_;
};

class EngineBuilder {
 public:
  EngineBuilder& with_debounce_window(std::chrono::milliseconds window);
  EngineBuilder& add_route(const config::RouteConfig& route);

  std::expected<Engine, std::string> build();

 private:
  config::VoyConfig config_;
};

}  // namespace voy::engine
