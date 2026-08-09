#include <voy/event.hpp>

#include <doctest/doctest.h>
#include <sys/inotify.h>

using namespace voy::event;

TEST_CASE("EventType String Conversion (Round-Trip)") {
  auto round_trip  = [&](EventType type) { return from_string(to_string(type)) == type; };
  auto event_types = {
      EventType::Modify,     EventType::Create,    EventType::Delete,  EventType::Attrib,
      EventType::CloseWrite, EventType::MovedFrom, EventType::MovedTo, EventType::DeleteSelf,
      EventType::MoveSelf,   EventType::All,       EventType::None,
  };
  for (auto type : event_types) {
    CHECK(round_trip(type));
  }
}
