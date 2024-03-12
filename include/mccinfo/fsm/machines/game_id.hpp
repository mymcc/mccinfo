#pragma once

#include "mccinfo/fsm/states/game_id_states.hpp"

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

        boost::sml::state<halo3> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<halo3> + event<mcc_terminate> = boost::sml::state<none>,

        boost::sml::state<halo3odst> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<halo3odst> + event<mcc_terminate> = boost::sml::state<none>,

        boost::sml::state<haloreach> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<haloreach> + event<mcc_terminate> = boost::sml::state<none>,

        boost::sml::state<halo4> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<halo4> + event<mcc_terminate> = boost::sml::state<none>,

        boost::sml::state<halo2a> + event<game_exit> = boost::sml::state<none>,
        boost::sml::state<halo2a> + event<mcc_terminate> = boost::sml::state<none>,

        CALLBACK_ON_TRANSITION(boost::sml::state<none>, NONE),
        CALLBACK_ON_TRANSITION(boost::sml::state<haloce>, HALOCE),
        CALLBACK_ON_TRANSITION(boost::sml::state<halo2>, HALO2),
        CALLBACK_ON_TRANSITION(boost::sml::state<halo3>, HALO3),
        CALLBACK_ON_TRANSITION(boost::sml::state<halo3odst>, HALO3ODST),
        CALLBACK_ON_TRANSITION(boost::sml::state<haloreach>, HALOREACH),
        CALLBACK_ON_TRANSITION(boost::sml::state<halo4>, HALO4),
        CALLBACK_ON_TRANSITION(boost::sml::state<halo2a>, HALO2A)
    );
}
};
}
}
}