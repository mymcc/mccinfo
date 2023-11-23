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
        *boost::sml::state<eac_off> + event<eac_start> = boost::sml::state<eac_on>,
        boost::sml::state<eac_on> + event<eac_terminate> = boost::sml::state<eac_off>
    );
}
};
}
}
}