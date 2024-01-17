#pragma once

#include "mccinfo/fsm/events/mcc_events.hpp"
#include "mccinfo/fsm/events/user_events.hpp"

#include <variant>

namespace mccinfo {
namespace fsm {
namespace events {
	using event_t = std::variant<
		events::launcher_start,
		events::launcher_terminate,
		events::launch_complete,
		events::launch_abort,
		events::mcc_start,
		events::mcc_terminate,
		events::mcc_found,
		events::load_start,
		events::match_start,
		events::match_end,
		events::unload_end,
		events::in_menus_identified,
		events::match_found,
		events::match_paused>;
}
}
}