#pragma once

#include "state.hpp"
#include "mccinfo/utility.hpp"
#include "mccinfo/fsm/predicates.hpp"
#include "mccinfo/fsm/events/events.hpp"

namespace mccinfo {
namespace fsm {
namespace states {

MCCFSM_INLINE mcc_lossed__ = edges::make_sequence(&fsm::predicates::events::mcc_terminated);
MCCFSM_INLINE game_unloaded_ = edges::make_sequence(&predicates::events::restartscreen_gfx_file_created);
MCCFSM_INLINE haloce_starting = edges::make_sequence(&predicates::events::haloce_lang_bin_file_created);
MCCFSM_INLINE halo2_starting = edges::make_sequence(&predicates::events::halo2_lang_bin_file_created);
MCCFSM_INLINE halo2a_starting = edges::make_sequence(&predicates::events::halo2a_lang_bin_file_created);
MCCFSM_INLINE halo3_starting = edges::make_sequence(&predicates::events::halo3_lang_bin_file_created);
MCCFSM_INLINE halo3odst_starting = edges::make_sequence(&predicates::events::halo3odst_lang_bin_file_created);
MCCFSM_INLINE halo4_starting = edges::make_sequence(&predicates::events::halo4_lang_bin_file_created);
MCCFSM_INLINE haloreach_starting = edges::make_sequence(&predicates::events::haloreach_lang_bin_file_created);

struct none : public state<none> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&haloce_starting, events::haloce_found{}),
        std::make_tuple(&halo2_starting, events::halo2_found{}),
        std::make_tuple(&halo2a_starting, events::halo2a_found{}),
        std::make_tuple(&halo3_starting, events::halo3_found{}),
        std::make_tuple(&halo3odst_starting, events::halo3odst_found{}),
        std::make_tuple(&halo4_starting, events::halo4_found{}),
        std::make_tuple(&haloreach_starting, events::haloreach_found{})
    );
};

struct haloce : public state<haloce> {
	MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&game_unloaded_, events::game_exit{}),
        std::make_tuple(&mcc_lossed__, events::mcc_terminate{})
    );
};

struct halo2 : public state<halo2> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&game_unloaded_, events::game_exit{}),
        std::make_tuple(&mcc_lossed__, events::mcc_terminate{})
    );
};

struct halo2a : public state<halo2a> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&game_unloaded_, events::game_exit{}),
        std::make_tuple(&mcc_lossed__, events::mcc_terminate{})
    );
};

struct halo3 : public state<halo3> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&game_unloaded_, events::game_exit{}),
        std::make_tuple(&mcc_lossed__, events::mcc_terminate{})
    );
};

struct halo3odst : public state<halo3odst> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&game_unloaded_, events::game_exit{}),
        std::make_tuple(&mcc_lossed__, events::mcc_terminate{})
    );
};

struct halo4 : public state<halo4> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&game_unloaded_, events::game_exit{}),
        std::make_tuple(&mcc_lossed__, events::mcc_terminate{})
    );
};

struct haloreach : public state<haloreach> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&game_unloaded_, events::game_exit{}),
        std::make_tuple(&mcc_lossed__, events::mcc_terminate{})
    );
};

} // namespace states
} // namespace fsm
} // namespace mccinfo