#include <voy/event.hpp>

namespace voy {
std::string_view to_string(EventType type) noexcept {
  return (type == EventType::Modify) ? "modify" : "none";
}
}  // namespace voy
