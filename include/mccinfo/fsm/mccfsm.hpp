#pragma once

#include "machines/mcc.hpp"
#include "machines/launch.hpp"

#include "edges/transitions.hpp"
#include <iostream>
#include <string>
namespace mccinfo {
namespace fsm {
template <class = class Dummy> class controller {
  public:
    void handle_trace_event(const EVENT_RECORD &record,
                            const krabs::trace_context &trace_context) {
        utility::atomic_guard lk(lock);

        mcc_sm.visit_current_states([](auto state) {
            for (auto &kv : machines::mcc_edges) {

            }
        });
        launch_sm.visit_current_states([](auto state) {
            std::cout << utility::type_hash<decltype(state)>::name << std::endl;
        });
    }
  private:
    utility::atomic_mutex lock;
    //boost::sml::sm<machines::mcc, machines::launch> sm;

    boost::sml::sm<machines::mcc> mcc_sm;
    boost::sml::sm<machines::launch> launch_sm;
};
} // namespace fsm
} // namespace mccinfo