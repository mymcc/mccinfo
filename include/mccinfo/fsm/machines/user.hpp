#pragma once

#include "mccinfo/fsm/states/user_states.hpp"

namespace mccinfo {
namespace fsm {
namespace machines {

struct user {
    auto operator()() const {
    using namespace boost::sml;
    using namespace states;
    using namespace events;

    return make_transition_table(
        *boost::sml::state<offline> + event<mcc_start> = boost::sml::state<waiting_on_launch>,
        boost::sml::state<offline> + event<mcc_found> = boost::sml::state<identifying_session>,

        boost::sml::state<waiting_on_launch> + event<launch_complete> = boost::sml::state<in_menus>,
        boost::sml::state<waiting_on_launch> + event<mcc_terminate> = boost::sml::state<offline>,
        
        boost::sml::state<identifying_session> + event<launch_complete> = boost::sml::state<in_menus>,
        boost::sml::state<identifying_session> + event<launch_identified> = boost::sml::state<waiting_on_launch>,
        boost::sml::state<identifying_session> + event<in_menus_identified> = boost::sml::state<in_menus>,
        boost::sml::state<identifying_session> + event<match_found> = boost::sml::state<in_game>,
        boost::sml::state<identifying_session> + event<mcc_terminate> = boost::sml::state<offline>,

        boost::sml::state<in_menus> + event<load_start> = boost::sml::state<loading_in>,
        boost::sml::state<in_menus> + event<match_found> = boost::sml::state<loading_in>,
        boost::sml::state<in_menus> + event<mcc_terminate> = boost::sml::state<offline>,

        boost::sml::state<loading_in> + event<match_start> = boost::sml::state<in_game>,
        boost::sml::state<loading_in> + event<match_abort> = boost::sml::state<in_menus>,
        boost::sml::state<loading_in> + event<mcc_terminate> = boost::sml::state<offline>,

        boost::sml::state<in_game> + event<match_end> = boost::sml::state<loading_out>,
        boost::sml::state<in_game> + event<mcc_terminate> = boost::sml::state<offline>,

        boost::sml::state<loading_out> + event<load_start> = boost::sml::state<loading_in>,
        boost::sml::state<loading_out> + event<unload_end> = boost::sml::state<in_menus>,
        boost::sml::state<loading_out> + event<mcc_terminate> = boost::sml::state<offline>,

        CALLBACK_ON_TRANSITION(boost::sml::state<offline>, OFFLINE),
        CALLBACK_ON_TRANSITION(boost::sml::state<waiting_on_launch>, WAITING_ON_LAUNCH),
        CALLBACK_ON_TRANSITION(boost::sml::state<identifying_session>, IDENTIFYING_SESSION),
        CALLBACK_ON_TRANSITION(boost::sml::state<in_menus>, IN_MENUS),
        CALLBACK_ON_TRANSITION(boost::sml::state<loading_in>, LOADING_IN),
        CALLBACK_ON_TRANSITION(boost::sml::state<in_game>, IN_GAME),
        CALLBACK_ON_TRANSITION(boost::sml::state<loading_out>, LOADING_OUT)
    );
}
};
}
}
}