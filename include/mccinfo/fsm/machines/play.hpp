#pragma once

#include <boost/sml.hpp>

#include "mccinfo/query.hpp"
#include "mccinfo/fsm/states/play_states.hpp"

namespace mccinfo {
namespace fsm {
namespace machines {

struct play {
    auto operator()() const {
    using namespace boost::sml;
    using namespace states;
    using namespace events;

    return make_transition_table(
        *boost::sml::state<in_menus> + event<load_start> = boost::sml::state<loading_in>,
        //boost::sml::state<in_menus> + event<match_found> = boost::sml::state<in_match>,
        boost::sml::state<in_menus> + boost::sml::on_entry<_> / [] { std::cout << "in_menus\n" << std::flush; },
        boost::sml::state<loading_in> + event<match_start> = boost::sml::state<in_match>,
        boost::sml::state<loading_in> + boost::sml::on_entry<_> / [] { std::cout << "loading_in\n" << std::flush; },
        boost::sml::state<in_match> + event<match_end> = boost::sml::state<loading_out>,
        boost::sml::state<in_match> + boost::sml::on_entry<_> / [] { std::cout << "in_match\n" << std::flush; },
        boost::sml::state<loading_out> + event<unload_end> = boost::sml::state<in_menus>,
        boost::sml::state<loading_out> + boost::sml::on_entry<_> / [] { std::cout << "loading_out\n" << std::flush; }
    );
}
};
}
}
}