#pragma once

#include "state.hpp"
#include "..\..\utility.hpp"
#include "..\events\events.hpp"
#include "..\predicates.hpp"

namespace mccinfo {
namespace fsm {
namespace states {

MCCFSM_INLINE match_loading = edges::make_sequence(&predicates::init_txt_created);
MCCFSM_INLINE match_started = edges::make_sequence(&predicates::hud_scoring_created);
MCCFSM_INLINE match_ended = edges::make_sequence(&predicates::xml_tmp_created);
MCCFSM_INLINE match_underway = edges::make_sequence(&predicates::sound_file_read);

struct in_menus : public state<in_menus>{
	MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&match_loading, events::load_start{}),
        std::make_tuple(&match_started, events::match_found{}),
        std::make_tuple(&match_underway, events::match_found{})
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

} // namespace states
} // namespace fsm
} // namespace mccinfo