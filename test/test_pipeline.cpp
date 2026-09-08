/*
 * markings:managed
 *
 * File: test_pipeline.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#include <voy/config.hpp>
#include <voy/event.hpp>
#include <voy/pipeline.hpp>

#include <filesystem>
#include <unordered_map>
#include <vector>

#include <doctest/doctest.h>

TEST_CASE("Prepare environment returns empty env on no events") {
  voy::config::RouteConfig       route;
  std::vector<voy::event::Event> matched_events;

  auto env = voy::pipeline::Pipeline::prepare_environment(route, matched_events);
  CHECK(env.size() == 0);
}

TEST_CASE("Prepare environment still passes through action env on no events") {
  voy::config::RouteConfig       route;
  std::vector<voy::event::Event> matched_events;

  route.action.env["FOO"] = "BAR";

  auto env = voy::pipeline::Pipeline::prepare_environment(route, matched_events);
  CHECK(env.size() == 1);
  CHECK(env["FOO"] == "BAR");
}

TEST_CASE("Prepare environment correctly reports single event") {
  voy::config::RouteConfig       route;
  std::vector<voy::event::Event> matched_events;

  voy::event::Event event;
  event.path = std::filesystem::path("test.txt");
  event.type = voy::event::EventType::Modify;

  matched_events.push_back(event);

  auto env = voy::pipeline::Pipeline::prepare_environment(route, matched_events);
  CHECK(env["VOY_BATCH_SIZE"] == "1");
  CHECK(env["VOY_EVENT_PATH"] == "test.txt");
  CHECK(env["VOY_EVENT_TYPE"] == "modify");
}

TEST_CASE("Prepare environment correctly batches two file paths") {
  voy::config::RouteConfig       route;
  std::vector<voy::event::Event> matched_events;

  voy::event::Event event1;
  event1.path = std::filesystem::path("test_foo.txt");
  event1.type = voy::event::EventType::Modify;
  matched_events.push_back(event1);

  voy::event::Event event2;
  event2.path = std::filesystem::path("test_bar.txt");
  event2.type = voy::event::EventType::Create;
  matched_events.push_back(event2);

  auto env = voy::pipeline::Pipeline::prepare_environment(route, matched_events);
  CHECK(env["VOY_BATCH_SIZE"] == "2");
  CHECK(env["VOY_EVENT_PATH"] == "test_foo.txt:test_bar.txt");
  CHECK(env["VOY_EVENT_TYPE"] == "modify|create");
}

TEST_CASE("Prepare environment correctly batches multiple file paths") {
  voy::config::RouteConfig       route;
  std::vector<voy::event::Event> matched_events{
      voy::event::Event{.path = "test_foo.txt", .type = voy::event::EventType::Create},
      voy::event::Event{.path = "test_bar.txt", .type = voy::event::EventType::Create},
      voy::event::Event{.path = "test_fizz.txt", .type = voy::event::EventType::Create},
      voy::event::Event{.path = "test_buzz.txt", .type = voy::event::EventType::Create},
      voy::event::Event{.path = "test_bazz.txt", .type = voy::event::EventType::Create},
  };

  auto env = voy::pipeline::Pipeline::prepare_environment(route, matched_events);
  CHECK(env["VOY_BATCH_SIZE"] == "5");
  CHECK(env["VOY_EVENT_PATH"] ==
        "test_foo.txt:test_bar.txt:test_fizz.txt:test_buzz.txt:test_bazz.txt");
  CHECK(env["VOY_EVENT_TYPE"] == "create");
}
