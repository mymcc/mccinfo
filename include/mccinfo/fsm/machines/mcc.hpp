#pragma once

#include <boost/sml.hpp>

#include "..\..\query.hpp"
#include "..\states\mcc_states.hpp"

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
        *boost::sml::state<off> + event<launcher_start> /*[!can_find_mcc]*/ = boost::sml::state<launching>,
        boost::sml::state<off> + event<mcc_found> = boost::sml::state<on>,
        boost::sml::state<launching> + event<launch_complete> /*[can_find_mcc]*/ = boost::sml::state<on>,
        boost::sml::state<launching> + event<launch_abort> = boost::sml::state<off>,
        boost::sml::state<on> + event<mcc_terminate> = boost::sml::state<off>,

        boost::sml::state<off> + boost::sml::on_entry<_> / [] { std::cout << "off\n" << std::flush; },
        boost::sml::state<launching> + boost::sml::on_entry<_> / [] { std::cout << "launching\n" << std::flush; },
        boost::sml::state<on> + boost::sml::on_entry<_> / [] { std::cout << "on\n" << std::flush; }
    );
}
};
}
}
}