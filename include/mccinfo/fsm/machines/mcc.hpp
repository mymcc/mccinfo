#pragma once

#include "mccinfo/fsm/states/mcc_states.hpp"

namespace mccinfo {
namespace fsm {
namespace machines {

struct mcc {
    auto operator()() const {
    using namespace boost::sml;
    using namespace states;
    using namespace events;

    return make_transition_table(
        *boost::sml::state<off> + event<launcher_start>  = boost::sml::state<launching>,
        boost::sml::state<off> + event<mcc_found> = boost::sml::state<on>,
        boost::sml::state<launching> + event<launch_complete> = boost::sml::state<on>,
        boost::sml::state<launching> + event<launch_abort> = boost::sml::state<off>,
        boost::sml::state<on> + event<mcc_terminate> = boost::sml::state<off>,

        CALLBACK_ON_TRANSITION(boost::sml::state<off>, OFF),
        CALLBACK_ON_TRANSITION(boost::sml::state<launching>, LAUNCHING),
        CALLBACK_ON_TRANSITION(boost::sml::state<on>, ON)
    );
}
};
}
}
}