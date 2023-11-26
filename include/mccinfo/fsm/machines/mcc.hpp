#pragma once
#define BOOST_SML_CREATE_DEFAULT_CONSTRUCTIBLE_DEPS
#include <boost/sml.hpp>
#include <cstdio>
#include <cassert>

#include <frozen/string.h>
#include <frozen/unordered_map.h>

#include "..\..\query.hpp"
#include "..\states\mcc_states.hpp"
#include "..\events\events.hpp"
#include "..\static_predicates.hpp"
#include "..\edges\sequences.hpp"
#include "..\edges\edges.hpp"
#include <any>
#include <string_view>
#include <variant>

namespace mccinfo {
namespace fsm {
namespace machines {

constexpr auto can_find_mcc = []() { return query::LookForMCCProcessID().has_value(); };

struct mcc {
    auto operator()() const {
    using namespace boost::sml;
    using namespace states;
    using namespace events;

    return make_transition_table(
        * boost::sml::state<off> + event<launcher_start> [!can_find_mcc] = boost::sml::state<launching>,
        boost::sml::state<launching> + event<launcher_terminate>[can_find_mcc] = boost::sml::state<on>,
        boost::sml::state<on> + event<mcc_terminate> = boost::sml::state<off>
    );
}
};
}
}
}