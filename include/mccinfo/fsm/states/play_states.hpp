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

MCCFSM_INLINE mm_bg_video_file_created = edges::make_sequence(&predicates::events::main_menu_background_video_file_created);
MCCFSM_INLINE in_menus_identifier = edges::make_sequence(&predicates::events::in_menus_video_file_read);
MCCFSM_INLINE mcc_lossed_ = edges::make_sequence(&fsm::predicates::events::mcc_terminated);

MCCFSM_INLINE match_underway = edges::make_sequence(&predicates::events::sound_file_read);


MCCFSM_INLINE mcc_started_ = edges::make_sequence(&predicates::events::mcc_started);
MCCFSM_INLINE mcc_found__ = edges::make_sequence(&predicates::events::mcc_found);
MCCFSM_INLINE match_paused_ = edges::make_sequence(&predicates::events::paused_game_gfx_file_created);


// works when players are launching when app is hot + when the last 2 or more consecutive matches are (halo3, ...?)
MCCFSM_INLINE no_match_underway = edges::make_sequence(
    &predicates::events::loaded_at_trace_start::wininet_image,
    &predicates::events::loaded_at_trace_start::wsock32_image
);

// needs to be a strict sequence
MCCFSM_INLINE potentially_match_underway = edges::make_sequence(
    &predicates::events::loaded_at_trace_start::cryptui_image,
    &predicates::certainly_not::wininet_image
);


struct offline : public state<offline>{
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&mcc_started_, events::mcc_start{}),
        std::make_tuple(&mcc_found__, events::mcc_found{})
    );
};

struct waiting_on_launch : public state<waiting_on_launch> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&mm_bg_video_file_created, events::launch_complete{}),
        std::make_tuple(&mcc_lossed_, events::mcc_terminate{})
    );
};

struct identifying_session : public state<identifying_session> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&mm_bg_video_file_created, events::launch_complete{}),
        std::make_tuple(&no_match_underway, events::in_menus_identified{}),
        std::make_tuple(&mcc_lossed_, events::mcc_terminate{})
    );
};

struct identifying_match_state : public state<identifying_match_state> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&match_paused_, events::match_paused{}),
        std::make_tuple(&mcc_lossed_, events::mcc_terminate{})
    );
};

struct in_menus : public state<in_menus>{
	MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&match_loading, events::load_start{}),
        std::make_tuple(&mcc_lossed_, events::mcc_terminate{})
    );
};

struct loading_in : public state<loading_in>{
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&match_started, events::match_start{}),
        std::make_tuple(&mcc_lossed_, events::mcc_terminate{})
    );
};

struct in_match : public state<in_match>{
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&match_ended, events::match_end{}),
        std::make_tuple(&mcc_lossed_, events::mcc_terminate{})
    );
};

struct loading_out : public state<loading_out>{
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&match_unloaded, events::unload_end{}),
        std::make_tuple(&mcc_lossed_, events::mcc_terminate{})
    );
};

} // namespace states
} // namespace fsm
} // namespace mccinfo