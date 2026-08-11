#pragma once

#include <voy/event.hpp>
#include <voy/reactor.hpp>

#include <algorithm>
#include <chrono>
#include <expected>
#include <functional>
#include <unordered_map>
#include <vector>

namespace voy::debounce {

class Debouncer {
 public:
  using FlushCallback = std::function<void(std::vector<event::Event>)>;

  [[nodiscard]] static std::expected<Debouncer, std::string> create(
      std::chrono::milliseconds window);

  ~Debouncer();

  Debouncer(const Debouncer&)            = delete;
  Debouncer& operator=(const Debouncer&) = delete;

  Debouncer(Debouncer&& other) noexcept;
  Debouncer& operator=(Debouncer&& other) noexcept;

  void on_raw_event(const event::Event& event);
  void on_timer_expired();
  void set_flush_callback(FlushCallback cb) { flush_cb_ = std::move(cb); }

  [[nodiscard]] int timer_fd() const noexcept { return timer_fd_; }

 private:
  Debouncer(int timer_fd, std::chrono::milliseconds window);

  int                                           timer_fd_{-1};
  std::chrono::milliseconds                     window_;
  FlushCallback                                 flush_cb_;
  std::unordered_map<std::string, event::Event> buffer_;
};

}  // namespace voy::debounce
