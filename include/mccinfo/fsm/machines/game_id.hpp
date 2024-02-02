#pragma once

#include <boost/sml.hpp>

#include "mccinfo/query.hpp"
#include "mccinfo/fsm/states/game_id_states.hpp"
#include "mccinfo/fsm/callback_table.hpp"

namespace mccinfo {
namespace fsm {
namespace machines {

struct game_id {
    auto operator()() const {
    using namespace boost::sml;
    using namespace states;
    using namespace events;

    return make_transition_table(
        *boost::sml::state<none> + event<haloce_found> = boost::sml::state<haloce>,
        boost::sml::state<none> + event<halo2_found> = boost::sml::state<halo2>,
        boost::sml::state<none> + event<halo2a_found> = boost::sml::state<halo2a>,
        boost::sml::state<none> + event<halo3_found> = boost::sml::state<halo3>,
        boost::sml::state<none> + event<halo3odst_found> = boost::sml::state<halo3odst>,
        boost::sml::state<none> + event<halo4_found> = boost::sml::state<halo4>,
        boost::sml::state<none> + event<haloreach_found> = boost::sml::state<haloreach>,

        boost::sml::state<haloce> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<haloce> + event<mcc_terminate> = boost::sml::state<none>,

        boost::sml::state<halo2> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<halo2> + event<mcc_terminate> = boost::sml::state<none>,

        boost::sml::state<halo2a> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<halo2a> + event<mcc_terminate> = boost::sml::state<none>,

        boost::sml::state<halo3> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<halo3> + event<mcc_terminate> = boost::sml::state<none>,

        boost::sml::state<halo3odst> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<halo3odst> + event<mcc_terminate> = boost::sml::state<none>,

        boost::sml::state<halo4> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<halo4> + event<mcc_terminate> = boost::sml::state<none>,

        boost::sml::state<haloreach> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<haloreach> + event<mcc_terminate> = boost::sml::state<none>,


        boost::sml::state<none> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "none\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | NONE);
                                     },
        boost::sml::state<haloce> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "haloce\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | HALOCE);
                                     },
        boost::sml::state<halo2> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "halo2\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | HALO2);
                                     },
        boost::sml::state<halo2a> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "halo2a\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | HALO2A);
                                     },
        boost::sml::state<halo3> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "halo3\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | HALO3);
                                     },
        boost::sml::state<halo3odst> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "halo3odst\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | HALO3ODST);
                                     },
        boost::sml::state<halo4> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "halo4\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | HALO4);
                                     },
        boost::sml::state<haloreach> + boost::sml::on_entry<_> /
                                     [](callback_table& cbtable) {
                                         std::cout << "haloreach\n" << std::flush;
                                         cbtable.execute_callback(ON_STATE_ENTRY | HALOREACH);
                                     }
    );
}
};
}
}
}