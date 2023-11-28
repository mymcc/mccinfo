#pragma once

#include "mcc_events.hpp"
#include "launch_events.hpp"
#include <variant>

namespace mccinfo {
namespace fsm {
namespace events {
	using event_t = std::variant<
		events::launcher_start,
		events::launcher_terminate,
		events::mcc_start,
		events::mcc_terminate,
		events::mcc_found>;
}
}
}