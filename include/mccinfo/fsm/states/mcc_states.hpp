#pragma once

#include "state.hpp"
#include "..\..\utility.hpp"
#include "..\events\events.hpp"
#include "..\static_predicates.hpp"

namespace mccinfo {
namespace fsm {
namespace states {

MCCFSM_INLINE seq1 =
    edges::make_sequence(&predicates::launcher_start_pred, &predicates::eac_start_pred);

MCCFSM_INLINE seq2 =
    edges::make_sequence(&predicates::mcc_process_start, &predicates::launcher_end_pred);

MCCFSM_INLINE seq3 =
    edges::make_sequence(&fsm::predicates::mcc_process_lossed);

struct off : public state<off> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&seq1, events::launcher_start{})
    );
};

struct launching : public state<launching> {
    MCCFSM_STATIC edges = edges::make_edges( 
        std::make_tuple(&seq2, events::launcher_terminate{})
    );
};

struct on : public state<on> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&seq3, events::mcc_terminate{})
    );
};
} // namespace states
} // namespace fsm
} // namespace mccinfo