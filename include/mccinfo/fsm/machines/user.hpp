#pragma once

#include <boost/sml.hpp>

#include "mccinfo/query.hpp"
#include "mccinfo/fsm/states/user_states.hpp"
#include "mccinfo/fsm/callback_table.hpp"
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

        boost::sml::state<offline> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "offline\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | OFFLINE);
                                     },
        boost::sml::state<waiting_on_launch> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "waiting_on_launch\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | WAITING_ON_LAUNCH);
                                     },
        boost::sml::state<identifying_session> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "identifying_session\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | IDENTIFYING_SESSION);
                                     },
        boost::sml::state<in_menus> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "in_menus\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | IN_MENUS);
                                     },
        boost::sml::state<loading_in> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "loading_in\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | LOADING_IN);
                                     },
        boost::sml::state<in_game> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "in_match\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | IN_GAME);
                                     },
        boost::sml::state<loading_out> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "loading_out\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | LOADING_OUT);
                                     }

    );
}
};
}
}
}