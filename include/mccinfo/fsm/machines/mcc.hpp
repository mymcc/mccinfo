#pragma once

#include <boost/sml.hpp>
#include <cstdio>
#include <cassert>
#include "..\..\query.hpp"
#include "..\states\mcc_states.hpp"
#include "..\events\mcc_events.hpp"
#include "..\static_predicates.hpp"
#include "..\transitions\sequences.hpp"
#include "..\transitions\triggers.hpp"



namespace mccinfo {
namespace fsm {
namespace machines {

static auto seq1 = fsm::predicates::details::make_sequence<events::launcher_start>(
    &fsm2::predicates::launcher_start_pred);

static auto seq2 = fsm::predicates::details::make_sequence<events::launcher_start>(
    &fsm2::predicates::launcher_end_pred);

static auto trigger1 = transitions::make_trigger<events::launcher_start>(&seq1);
static auto trigger2 = transitions::make_trigger<events::launcher_terminate>(&seq2);

constexpr auto can_find_mcc = []() { return query::LookForMCCProcessID().has_value(); };

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