#pragma once

#include "..\query.hpp"

#include "static_predicates.hpp"
#include "predicate_sequence.hpp"
#include "events.hpp"
#include "states.hpp"

#include <iostream>
#include <thread>

namespace mccinfo {
namespace fsm2 {
namespace provider {

    static ctfsm::fsm <fsm2::states::mcc_initial>* s_fsm = fsm2::states::mcc_fsm::instance<fsm2::states::mcc_initial>();

    void process_rundown_callback(const EVENT_RECORD& record,
        const krabs::trace_context& trace_context);

    bool StartETW(void) {

        // we will make different filters, attach different callbacks that dispatch their respective
        // event. like a successful predicate filter should elicit next_predicate_in_seq but a process
        // end for mcc should elicit terminate, but only if its a pid we recognized to be on and were
        // tracking
        s_fsm->handle_event<fsm2::events::start>();

        if (query::LookForMCCProcessID().has_value()) {
            std::cout << "MCC PID found" << std::endl;
            // forcing state through edge traversal (perhaps sml/sml2 will not force us to do this)
            s_fsm->handle_event<fsm2::events::launcher_start>();
            s_fsm->handle_event<fsm2::events::launcher_end>();
        }


        krabs::kernel_trace trace(L"kernel_trace");
        krabs::kernel::process_provider process_provider;

        krabs::event_filter filter { krabs::predicates::any_of({ &predicates::is_launcher, &predicates::is_eac, &predicates::is_mcc }) };

        filter.add_on_event_callback(process_rundown_callback);
        process_provider.add_filter(filter);
        trace.enable(process_provider);

        // process_provider.add_filter

        // krabs::kernel::disk_file_io_provider file_io_provider;
        // file_io_provider.add_on_event_callback(file_rundown_callback);
        ////trace.enable(file_io_provider);
        //
        // krabs::kernel_provider hwconfig_provider(0, krabs::guids::event_trace_config);
        // hwconfig_provider.add_on_event_callback(hwconfig_callback);
        // trace.enable(hwconfig_provider);

        // krabs::kernel_trace trace2(L"kernel_trace2");
        // krabs::kernel::disk_file_io_provider

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

    void process_rundown_callback(const EVENT_RECORD& record,
        const krabs::trace_context& trace_context) {

        s_fsm->invoke_on_current([&](auto&& current, auto& fsm) {
            current.handle_trace_event<decltype(s_fsm)>(&s_fsm, record, trace_context);
        });
    }
}
}
}