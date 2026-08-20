#include <voy/debounce.hpp>
#include <voy/event.hpp>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

#include <sys/time.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace voy::debounce {

std::expected<Debouncer, std::string> Debouncer::create(std::chrono::milliseconds window) {
  int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timer_fd == -1) {
    return std::unexpected("timerfd_create failed: " + std::string(strerror(errno)));
  }

  return Debouncer(timer_fd, window);
}

Debouncer::Debouncer(int timer_fd, std::chrono::milliseconds window)
    : timer_fd_(timer_fd), window_(window) {}

Debouncer::~Debouncer() {
  if (timer_fd_ != -1) close(timer_fd_);
}

Debouncer::Debouncer(Debouncer&& other) noexcept
    : timer_fd_(std::exchange(other.timer_fd_, -1)),
      window_(other.window_),
      flush_cb_(std::move(other.flush_cb_)),
      buffer_(std::move(other.buffer_)) {}

Debouncer& Debouncer::operator=(Debouncer&& other) noexcept {
  if (this != &other) {
    if (timer_fd_ != -1) close(timer_fd_);

    timer_fd_ = std::exchange(other.timer_fd_, -1);
    window_   = other.window_;
    flush_cb_ = std::move(other.flush_cb_);
    buffer_   = std::move(other.buffer_);
  }
  return *this;
}

void Debouncer::on_raw_event(const event::Event& event) {
  std::string path_str = event.path.string();
  auto        it       = buffer_.find(path_str);

  if (it != buffer_.end()) {
    it->second.type |= event.type;
    it->second.raw_inotify_mask |= event.raw_inotify_mask;
    it->second.timestamp = event.timestamp;
  } else {
    buffer_[path_str] = event;
  }

  struct itimerspec ts{};

  ts.it_value.tv_sec  = window_.count() / 1000;
  ts.it_value.tv_nsec = (window_.count() % 1000) * 1000000;

  if (timerfd_settime(timer_fd_, 0, &ts, nullptr) == -1) {
    // TODO: handle invalid args to syscall
  }
}

void Debouncer::on_timer_expired() {
  uint64_t expirations;
  if (read(timer_fd_, &expirations, sizeof(expirations)) == -1) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) return;
  }

  if (flush_cb_ && !buffer_.empty()) {
    std::vector<event::Event> flushed_events;
    flushed_events.reserve(buffer_.size());

    for (auto& [_, ev] : buffer_) {
      flushed_events.push_back(std::move(ev));
    }

    buffer_.clear();
    flush_cb_(std::move(flushed_events));
  }
}

}  // namespace voy::debounce
