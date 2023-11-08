#pragma once

#include <boost/sml.hpp>
#include <cstdio>
#include <cassert>

#include <frozen/string.h>
#include <frozen/unordered_map.h>

#include "..\..\query.hpp"
#include "..\states\mcc_states.hpp"
#include "..\events\mcc_events.hpp"
#include "..\static_predicates.hpp"
#include "..\edges\sequences.hpp"
#include "..\edges\transitions.hpp"
#include <any>
#include <string_view>
namespace mccinfo {
namespace fsm {
namespace machines {
namespace {
inline constinit auto seq1 =
    fsm::predicates::details::make_sequence(&fsm2::predicates::launcher_start_pred);
inline constinit auto seq2 =
    fsm::predicates::details::make_sequence(&fsm2::predicates::launcher_end_pred);

inline constinit auto mcc_edges =
    edges::make_edges(edges::make_proxy(&seq1)->*edges::event_id<events::launcher_start>,
                      edges::make_proxy(&seq2)->*edges::event_id<events::launcher_terminate>);

inline constinit auto mcc_edge_events = edges::make_edge_events(edges::event_<events::launcher_start>);

constexpr auto can_find_mcc = []() { return query::LookForMCCProcessID().has_value(); };
} // namespace

struct mcc {
    auto operator()() const {
    using namespace boost::sml;
    using namespace states;
    using namespace events;

    return make_transition_table(
        *state<off> + event<launcher_start> [!can_find_mcc] = state<launching>,
        state<launching> + event<launcher_terminate>[can_find_mcc] = state<on>,
        state<on> + event<mcc_terminate> = state<off>
    );
}
};
}
}
}