#pragma once

#include <boost/sml.hpp>
#include <cstdio>
#include <cassert>

namespace mccinfo {
namespace fsm {
    
class off;
class launching;
class on;

struct launcher_start {
    bool valid{};
};

struct launcher_terminate {
    bool valid{};
};
struct mcc_terminate {bool valid{};};

constexpr auto is_valid = [](const auto& event) { return event.valid; };

const auto send_fin = [] {};
const auto send_ack = [] {};

struct mcc {
    auto operator()() const {
        using namespace boost::sml;
        return make_transition_table(*state<off> + event<launcher_start> / send_fin = state<launching>, 
            state<launching> + event<launcher_terminate> [ is_valid ] = state<on>,
            state<on> + event<mcc_terminate> = state<off>
        );
    }
};

/*
struct release {};
struct ack {};
struct fin {};
struct timeout {};

const auto is_ack_valid = [](const ack&) { return true; };
const auto is_fin_valid = [](const fin&) { return true; };

const auto send_fin = [] {};
const auto send_ack = [] {};

class established;
class fin_wait_1;
class fin_wait_2;
class timed_wait;

struct hello_world {
  auto operator()() const {
    using namespace sml;
    return make_transition_table(
      *state<established> + event<release> / send_fin = state<fin_wait_1>,
       state<fin_wait_1> + event<ack> [ is_ack_valid ] = state<fin_wait_2>,
       state<fin_wait_2> + event<fin> [ is_fin_valid ] / send_ack = state<timed_wait>,
       state<timed_wait> + event<timeout> / send_ack = X
    );
  }
};*/
}
}