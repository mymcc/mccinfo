#pragma once

#include "..\query.hpp"
#include "mccfsm.hpp"
#include "static_predicates.hpp"
#include "fsm/fsm.hpp"

#include <iostream>
#include <thread>

namespace mccinfo {
namespace fsm {
namespace provider {


bool StartETW(void) {
    using namespace boost::sml;
    
    fsm::controller<> sm{};
    sm.initialize();

    krabs::kernel_trace trace(L"kernel_trace");
    krabs::kernel::process_provider process_provider;

    krabs::event_filter filter{krabs::predicates::any_of(
        {&predicates::is_launcher, &predicates::is_eac, &predicates::is_mcc})};

    filter.add_on_event_callback([&sm](const EVENT_RECORD &record,
                              const krabs::trace_context &trace_context){
            sm.handle_trace_event(record, trace_context);
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
} // namespace fsm
} // namespace mccinfo