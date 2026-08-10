#include "voy/event.hpp"

#include <voy/reactor.hpp>

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstring>
#include <utility>

#include <bits/types/sigset_t.h>
#include <linux/limits.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <unistd.h>

namespace voy::reactor {
namespace {
constexpr int    MAX_EPOLL_EVENTS = 10;
constexpr size_t INOTIFY_BUF_LEN  = 4096 * (sizeof(struct inotify_event) + NAME_MAX + 1);
}  // namespace

std::expected<Reactor, std::string> Reactor::create() {
  int inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  if (inotify_fd < 0) {
    return std::unexpected("inotify_init1 failed: " + std::string(strerror(errno)));
  }

  int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd < 0) {
    close(inotify_fd);
    return std::unexpected("epoll_create1 failed: " + std::string(strerror(errno)));
  }

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTERM);

  if (sigprocmask(SIG_BLOCK, &mask, nullptr) == 1) {
    close(inotify_fd);
    close(epoll_fd);
    return std::unexpected("sigprocmask failed: " + std::string(strerror(errno)));
  }

  int signal_fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
  if (signal_fd < 0) {
    close(inotify_fd);
    close(epoll_fd);
    return std::unexpected("signalfd failed: " + std::string(strerror(errno)));
  }

  struct epoll_event ev_inotify{};
  ev_inotify.events  = EPOLLIN;
  ev_inotify.data.fd = inotify_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inotify_fd, &ev_inotify) == -1) {
    return std::unexpected("epoll_ctl (inotify) failed: " + std::string(strerror(errno)));
  }

  struct epoll_event ev_signal{};
  ev_signal.events  = EPOLLIN;
  ev_signal.data.fd = signal_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, signal_fd, &ev_signal) == -1) {
    return std::unexpected("epoll_ctl (signal) failed: " + std::string(strerror(errno)));
  }

  return Reactor(inotify_fd, epoll_fd, signal_fd);
}

Reactor::Reactor(int inotify_fd, int epoll_fd, int signal_fd)
    : inotify_fd_(inotify_fd), epoll_fd_(epoll_fd), signal_fd_(signal_fd) {}

Reactor::~Reactor() {
  if (inotify_fd_ != -1) close(inotify_fd_);
  if (epoll_fd_ != -1) close(epoll_fd_);
  if (signal_fd_ != -1) close(signal_fd_);
}

Reactor::Reactor(Reactor&& other) noexcept
    : inotify_fd_(std::exchange(other.inotify_fd_, -1)),
      epoll_fd_(std::exchange(other.epoll_fd_, -1)),
      signal_fd_(std::exchange(other.signal_fd_, -1)),
      running_(std::exchange(other.running_, false)),
      wd_to_path_(other.wd_to_path_) {}

Reactor& Reactor::operator=(Reactor&& other) noexcept {
  if (this != &other) {
    if (inotify_fd_ != -1) close(inotify_fd_);
    if (epoll_fd_ != -1) close(epoll_fd_);
    if (signal_fd_ != -1) close(signal_fd_);

    inotify_fd_ = std::exchange(other.inotify_fd_, -1);
    epoll_fd_   = std::exchange(other.epoll_fd_, -1);
    signal_fd_  = std::exchange(other.signal_fd_, -1);
    running_    = std::exchange(other.running_, false);
    wd_to_path_ = std::move(other.wd_to_path_);
  }
  return *this;
}

std::expected<int, std::string> Reactor::add_watch(const std::string& dir, event::EventType mask) {
  int wd = inotify_add_watch(inotify_fd_, dir.c_str(), event::to_inotify_mask(mask));
  if (wd < 0) {
    return std::unexpected("Failed to watch directory '" + dir + "': " + strerror(errno));
  }

  wd_to_path_[wd] = dir;
  return wd;
}

void Reactor::stop() {
  running_ = false;
}

void Reactor::run(EventCallback on_event) {
  running_ = true;
  struct epoll_event events[MAX_EPOLL_EVENTS];

  while (running_) {
    int nfds = epoll_wait(epoll_fd_, events, MAX_EPOLL_EVENTS, -1);

    if (nfds == -1) {
      if (errno == EINTR) continue;
      break;
    }

    for (int i = 0; i < nfds; ++i) {
      if (events[i].data.fd == inotify_fd_) {
        handle_inotify_events(on_event);
      } else if (events[i].data.fd == signal_fd_) {
        struct signalfd_siginfo siginfo;
        if (read(signal_fd_, &siginfo, sizeof(siginfo)) == sizeof(siginfo)) { stop(); }
      }
    }
  }
}

void Reactor::handle_inotify_events(EventCallback& on_event) {
  alignas(struct inotify_event) char buffer[INOTIFY_BUF_LEN];

  while (true) {
    ssize_t len = read(inotify_fd_, buffer, sizeof(buffer));

    if (len == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) { break; }
      return;
    }

    for (char* ptr = buffer; ptr < buffer + len;) {
      auto* event = reinterpret_cast<struct inotify_event*>(ptr);

      if (event->mask & IN_Q_OVERFLOW) {
        // TODO: trigger full resync
      } else if (event->len > 0) {
        auto it = wd_to_path_.find(event->wd);
        if (it != wd_to_path_.end()) {
          event::Event voy_event;

          std::filesystem::path dir_path = it->second;
          voy_event.path                 = dir_path / event->name;
          voy_event.type                 = event::from_inotify_mask(event->mask);
          voy_event.raw_inotify_mask     = event->mask;
          voy_event.cookie               = event->cookie;
          voy_event.timestamp            = std::chrono::system_clock::now();

          on_event(voy_event);
        }
      }

      ptr += sizeof(struct inotify_event) + event->len;
    }
  }
}

}  // namespace voy::reactor
