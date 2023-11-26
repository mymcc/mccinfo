#pragma once

#include "state.hpp"
#include "..\..\utility.hpp"
#include "..\events\events.hpp"
#include "..\static_predicates.hpp"

namespace mccinfo {
namespace fsm {
namespace states {

MCCFSM_INLINE seq1 =
    edges::make_sequence(&fsm2::predicates::launcher_start_pred);

MCCFSM_INLINE seq2 =
    edges::make_sequence(&fsm2::predicates::launcher_end_pred);

struct off : public state<off> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&seq1, events::launcher_start{}), 
        std::make_tuple(&seq2, events::launcher_terminate{})
    );
};

struct launching : public state<launching> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&seq1, events::launcher_start{}), 
        std::make_tuple(&seq2, events::launcher_terminate{})
    );

};

struct on : public state<on> {
    MCCFSM_STATIC edges = edges::make_edges(
        std::make_tuple(&seq1, events::launcher_start{}), 
        std::make_tuple(&seq2, events::launcher_terminate{})
    );

};
} // namespace states
} // namespace fsm
} // namespace mccinfo