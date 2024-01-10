#pragma once

#include "state.hpp"
#include "mccinfo/utility.hpp"
#include "mccinfo/fsm/predicates.hpp"
#include "mccinfo/fsm/events/events.hpp"

namespace mccinfo {
namespace fsm {
namespace states {

MCCFSM_INLINE launcher_started = edges::make_sequence(&predicates::launcher_start_pred);
MCCFSM_INLINE launcher_found = edges::make_sequence(&predicates::mcc_launcher_found);
MCCFSM_INLINE mcc_started = edges::make_sequence(&predicates::mcc_process_start);
MCCFSM_INLINE mcc_found_ = edges::make_sequence(&predicates::mcc_process_found);
MCCFSM_INLINE main_menu_bg_created = edges::make_sequence(&predicates::main_menu_bg_created);
MCCFSM_INLINE mcc_lossed = edges::make_sequence(&fsm::predicates::mcc_process_lossed);

struct off : public state<off> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&launcher_started, events::launcher_start{}),
        std::make_tuple(&launcher_found, events::launcher_start{}),
        std::make_tuple(&mcc_started, events::launcher_start{}), 
        std::make_tuple(&mcc_found_, events::mcc_found{})
    );
};

struct launching : public state<launching> {
    MCCFSM_STATIC edges = edges::make_edges( 
        std::make_tuple(&main_menu_bg_created, events::launch_complete{}),
        std::make_tuple(&mcc_lossed, events::launch_abort{})
    );
};

struct on : public state<on> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&mcc_lossed, events::mcc_terminate{})
    );
};
} // namespace states
} // namespace fsm
} // namespace mccinfo