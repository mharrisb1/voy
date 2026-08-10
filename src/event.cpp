#include <voy/event.hpp>

#include <optional>

#include <sys/inotify.h>

namespace voy::event {

std::string_view to_string(EventType type) noexcept {
  switch (type) {
    case EventType::Modify: return "modify";
    case EventType::Create: return "create";
    case EventType::Delete: return "delete";
    case EventType::Attrib: return "attrib";
    case EventType::CloseWrite: return "close_write";
    case EventType::MovedFrom: return "moved_from";
    case EventType::MovedTo: return "moved_to";
    case EventType::DeleteSelf: return "delete_self";
    case EventType::MoveSelf: return "move_self";
    case EventType::All: return "all";
    case EventType::None:
    default: return "none";
  }
}

std::string to_composite_string(EventType mask) {
  if (mask == EventType::None) return "none";
  if (mask == EventType::All) return "all";

  std::string result;

  auto append_if = [&](EventType flag) {
    if (has_flag(mask, flag)) {
      if (!result.empty()) { result += '|'; }
      result.append(to_string(flag));
    }
  };

  append_if(EventType::Modify);
  append_if(EventType::Create);
  append_if(EventType::Delete);
  append_if(EventType::Attrib);
  append_if(EventType::CloseWrite);
  append_if(EventType::MovedFrom);
  append_if(EventType::MovedTo);
  append_if(EventType::MoveSelf);

  return result.empty() ? "none" : result;
}

std::optional<EventType> from_string(std::string_view str) noexcept {
  if (str == "modify") return EventType::Modify;
  if (str == "create") return EventType::Create;
  if (str == "delete") return EventType::Delete;
  if (str == "attrib") return EventType::Attrib;
  if (str == "close_write") return EventType::CloseWrite;
  if (str == "moved_from") return EventType::MovedFrom;
  if (str == "moved_to") return EventType::MovedTo;
  if (str == "delete_self") return EventType::DeleteSelf;
  if (str == "move_self") return EventType::MoveSelf;
  if (str == "all") return EventType::All;
  if (str == "none") return EventType::None;

  return std::nullopt;
}

std::optional<EventType> from_strings(std::vector<std::string>& strings) noexcept {
  EventType combined = EventType::None;
  for (const auto& s : strings) {
    auto parsed = from_string(s);
    if (!parsed.has_value()) { return std::nullopt; }
    combined |= *parsed;
  }
  return combined;
}

EventType from_inotify_mask(uint32_t mask) noexcept {
  EventType type = EventType::None;

  if (mask & IN_MODIFY) type |= EventType::Modify;
  if (mask & IN_CREATE) type |= EventType::Create;
  if (mask & IN_DELETE) type |= EventType::Delete;
  if (mask & IN_ATTRIB) type |= EventType::Attrib;
  if (mask & IN_CLOSE_WRITE) type |= EventType::CloseWrite;
  if (mask & IN_MOVED_FROM) type |= EventType::MovedFrom;
  if (mask & IN_MOVED_TO) type |= EventType::MovedFrom;
  if (mask & IN_DELETE_SELF) type |= EventType::DeleteSelf;
  if (mask & IN_MOVE_SELF) type |= EventType::MoveSelf;

  return type;
}

uint32_t to_inotify_mask(EventType type) noexcept {
  if (type == EventType::All) {
    return IN_MODIFY | IN_CREATE | IN_DELETE | IN_ATTRIB | IN_CLOSE_WRITE | IN_MOVED_FROM |
           IN_MOVED_TO | IN_DELETE_SELF | IN_MOVE_SELF;
  }

  uint32_t mask = 0;
  if (has_flag(type, EventType::Modify)) mask |= IN_MODIFY;
  if (has_flag(type, EventType::Create)) mask |= IN_CREATE;
  if (has_flag(type, EventType::Delete)) mask |= IN_DELETE;
  if (has_flag(type, EventType::Attrib)) mask |= IN_ATTRIB;
  if (has_flag(type, EventType::CloseWrite)) mask |= IN_CLOSE_WRITE;
  if (has_flag(type, EventType::MovedFrom)) mask |= IN_MOVED_FROM;
  if (has_flag(type, EventType::MovedTo)) mask |= IN_MOVED_TO;
  if (has_flag(type, EventType::DeleteSelf)) mask |= IN_DELETE_SELF;
  if (has_flag(type, EventType::MoveSelf)) mask |= IN_MOVE_SELF;

  return mask;
}

}  // namespace voy::event
