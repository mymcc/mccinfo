#pragma once
//#define BOOST_SML_CREATE_DEFAULT_CONSTRUCTIBLE_DEPS
#include "machines/mcc.hpp"
#include "machines/launch.hpp"

#include "edges/edges.hpp"
#include <iostream>
#include <string>
namespace mccinfo {
namespace fsm {
template <class = class Dummy> class controller {
  public:
    void handle_trace_event(const EVENT_RECORD &record,
                            const krabs::trace_context &trace_context) {
        utility::atomic_guard lk(lock);

        auto visit = [&](auto state) {
            states::BonusStateVisitor<decltype(mcc_sm)> visitor(mcc_sm, record, trace_context);
            mcc_sm.visit_current_states(visitor);
        };
        mcc_sm.visit_current_states(visit);

    }
  private:
    utility::atomic_mutex lock;
    //boost::sml::sm<machines::mcc, machines::launch> sm;

    boost::sml::sm<machines::mcc> mcc_sm;
    //boost::sml::sm<machines::launch> launch_sm;
};
} // namespace fsm
} // namespace mccinfo