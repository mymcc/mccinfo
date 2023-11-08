#pragma once

#include "..\states\launch_states.hpp"
#include "..\events\launch_events.hpp"

namespace mccinfo {
namespace fsm {
namespace machines {
struct launch {
	auto operator()() const {
    using namespace boost::sml;
    using namespace states;
    using namespace events;

    return make_transition_table(
        *state<eac_off> + event<eac_start> = state<eac_on>,
        state<eac_on> + event<eac_terminate> = state<eac_off>
    );
}
};
}
}
}