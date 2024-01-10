#pragma once

#include "state.hpp"
#include "mccinfo/utility.hpp"
#include "mccinfo/fsm/predicates.hpp"
#include "mccinfo/fsm/events/events.hpp"

namespace mccinfo {
namespace fsm {
namespace states {

MCCFSM_INLINE match_loading = edges::make_sequence(&predicates::events::match_init_file_created);
MCCFSM_INLINE match_started = edges::make_sequence(&predicates::events::hud_scoring_gfx_file_created);
MCCFSM_INLINE match_ended = edges::make_sequence(&predicates::events::temp_carnage_report_created);
MCCFSM_INLINE match_unloaded = edges::make_sequence(&predicates::events::restartscreen_gfx_file_created);
MCCFSM_INLINE match_underway = edges::make_sequence(&predicates::events::sound_file_read);

struct in_menus : public state<in_menus>{
	MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&match_loading, events::load_start{})
    );
};

struct loading_in : public state<loading_in>{
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&match_started, events::match_start{})
    );
};

struct in_match : public state<in_match>{
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&match_ended, events::match_end{})
    );
};

struct loading_out : public state<loading_out>{
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&match_unloaded, events::unload_end{})
    );
};

} // namespace states
} // namespace fsm
} // namespace mccinfo