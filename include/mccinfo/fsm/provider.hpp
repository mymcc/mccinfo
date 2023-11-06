#pragma once

#include "..\query.hpp"
#include "mccfsm.hpp"
#include "static_predicates.hpp"
#include "predicate_sequence.hpp"
#include "events.hpp"
#include "states.hpp"
#include "fsm/fsm.hpp"

#include <iostream>
#include <thread>

namespace mccinfo {
namespace fsm2 {
namespace provider {

bool StartETW(void) {
    using namespace boost::sml;
    sm<fsm::machines::mcc> sm;
    assert(sm.is(state<fsm::states::off>) == true);
    sm.process_event(fsm::events::launcher_start{});
    assert(sm.is(state<fsm::states::launching>) == true);
    fsm::transitions::trigger_handler th{ fsm::machines::trigger1, fsm::machines::trigger2 };
    ctfsm::fsm<fsm2::states::mcc_initial> fsm;
    fsm.handle_event<fsm2::events::start>();

    if (query::LookForMCCProcessID().has_value()) {
        std::cout << "MCC PID found" << std::endl;
        // forcing state through edge traversal (perhaps sml/sml2 will not force us to do this)
        fsm.handle_event<fsm2::events::launcher_start>();
        fsm.handle_event<fsm2::events::launcher_end>();
    }

    krabs::kernel_trace trace(L"kernel_trace");
    krabs::kernel::process_provider process_provider;

    krabs::event_filter filter{krabs::predicates::any_of(
        {&predicates::is_launcher, &predicates::is_eac, &predicates::is_mcc})};

    filter.add_on_event_callback([&fsm](const EVENT_RECORD &record,
                              const krabs::trace_context &trace_context){
            fsm.invoke_on_current([&](auto &&current, auto &_fsm) {
                current.handle_trace_event<decltype(fsm)>(&fsm, record, trace_context);
        });
    });
    
    process_provider.add_filter(filter);
    trace.enable(process_provider);

    std::cout << " - starting trace" << std::endl;

    std::thread thread([&trace]() { trace.start(); });

    while (true) {
        std::string input;
        std::getline(std::cin, input);
        if (input == "q") {
            std::cout << std::endl << " - stopping trace" << std::endl;
            trace.stop();
            break;
        }
    }

    thread.join();

    return true;
}
} // namespace provider
} // namespace fsm2
} // namespace mccinfo