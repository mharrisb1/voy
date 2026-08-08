#pragma once
#include <cstdint>
#include <string_view>

namespace voy {
enum class EventType : uint32_t { None = 0, Modify = 1 << 0 };
std::string_view to_string(EventType type) noexcept;
}  // namespace voy
