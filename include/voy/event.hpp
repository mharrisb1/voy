#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace voy::event {

enum class EventType : uint32_t {
  None       = 0,
  Modify     = 1 << 0,  // IN_MODIFY
  Create     = 1 << 1,  // IN_CREATE
  Delete     = 1 << 2,  // IN_DELETE
  Attrib     = 1 << 3,  // IN_ATTRIB
  CloseWrite = 1 << 4,  // IN_CLOSE_WRITE
  MovedFrom  = 1 << 5,  // IN_MOVED_FROM
  MovedTo    = 1 << 6,  // IN_MOVED_TO
  DeleteSelf = 1 << 7,  // IN_DELETE_SELF
  MoveSelf   = 1 << 8,  // IN_MOVE_SELF
  All        = 0xFFFFFFFF,
};

constexpr EventType operator|(EventType a, EventType b) noexcept {
  return static_cast<EventType>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

constexpr EventType operator&(EventType a, EventType b) noexcept {
  return static_cast<EventType>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

constexpr EventType operator^(EventType a, EventType b) noexcept {
  return static_cast<EventType>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}

constexpr EventType operator~(EventType a) noexcept {
  return static_cast<EventType>(~static_cast<uint32_t>(a));
}

constexpr EventType& operator|=(EventType& a, EventType b) noexcept {
  a = a | b;
  return a;
}

constexpr EventType& operator&=(EventType& a, EventType b) noexcept {
  a = a & b;
  return a;
}

consteval EventType& operator^=(EventType& a, EventType b) noexcept {
  a = a ^ b;
  return a;
}

[[nodiscard]] constexpr bool has_flag(EventType mask, EventType flag) noexcept {
  return (mask & flag) != EventType::None;
}

[[nodiscard]] std::string_view         to_string(EventType type) noexcept;
[[nodiscard]] std::string              to_composite_string(EventType mask);
[[nodiscard]] std::optional<EventType> from_string(std::string_view str) noexcept;
[[nodiscard]] std::optional<EventType> from_strings(std::vector<std::string>& strings) noexcept;
[[nodiscard]] EventType                from_inotify_mask(uint32_t mask) noexcept;
[[nodiscard]] uint32_t                 to_inotify_mask(EventType type) noexcept;

struct Event {
  std::filesystem::path                 path;
  EventType                             type{EventType::None};
  uint32_t                              raw_inotify_mask{0};
  uint32_t                              cookie{0};
  std::chrono::system_clock::time_point timestamp{std::chrono::system_clock::now()};
};

}  // namespace voy::event
