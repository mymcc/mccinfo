#pragma once

#include "mccinfo/fsm/events/mcc_events.hpp"
#include "mccinfo/fsm/events/user_events.hpp"
#include "mccinfo/fsm/events/game_id_events.hpp"

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
		events::match_abort,
		events::match_end,
		events::unload_end,
		events::launch_identified,
		events::in_menus_identified,
		events::match_found,
		events::haloce_found,
		events::halo2_found,
		events::halo2a_found,
		events::halo3_found,
		events::halo3odst_found,
		events::halo4_found,
		events::haloreach_found,
		events::game_exit,
		events::match_paused>;


inline event_t GetGameEventFromPath(const std::filesystem::path &path) {
    if      (utility::PathContains(path, "Halo1"))      return events::haloce_found{};
    else if (utility::PathContains(path, "Halo2A"))     return events::halo2a_found{};
    else if (utility::PathContains(path, "Halo2"))      return events::halo2_found{};
    else if (utility::PathContains(path, "Halo3ODST"))  return events::halo3odst_found{};
    else if (utility::PathContains(path, "Halo3"))      return events::halo3_found{};
    else if (utility::PathContains(path, "Halo4"))      return events::halo4_found{};
    else if (utility::PathContains(path, "HaloReach"))  return events::haloreach_found{};
    else throw std::runtime_error("GetGameEventFromPath(): game not found");
}

inline event_t GetGameEventFromHint(const game_hint& hint) {
    switch (hint) { 
    case game_hint::HALO1:      return events::haloce_found{};
    case game_hint::HALO2:      return events::halo2_found{};
    case game_hint::HALO3:      return events::halo3_found{};
    case game_hint::HALO3ODST:  return events::halo3odst_found{};
    case game_hint::HALOREACH:  return events::haloreach_found{};
    case game_hint::HALO4:      return events::halo4_found{};
    case game_hint::HALO2A:     return events::halo2a_found{};
    }
}
}
}
}