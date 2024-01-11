#pragma once

#include "mccinfo/query.hpp"
#include "mccinfo/fsm/mccfsm.hpp"
#include "mccinfo/fsm/predicates.hpp"

#include <iostream>
#include <thread>

namespace mccinfo {
namespace fsm {
namespace provider {

void print_fread(const EVENT_RECORD &record, const krabs::trace_context &trace_context) {
    krabs::schema schema(record, trace_context.schema_locator);
    krabs::parser parser(schema);
    auto opcode = schema.event_opcode();
    if (opcode == 67) {
        //if (io_size == (uint32_t)2048) {
        //if (true) {
        try {
            if (record.EventHeader.ProcessId == 24340) {
                uint32_t ttid = parser.parse<uint32_t>(L"TTID");
                uint32_t io_size = parser.parse<uint32_t>(L"IoSize");
                std::wcout << schema.task_name() << L"_" << schema.opcode_name();
                std::wcout << L" (" << schema.event_opcode() << L") ";
                std::wcout << L" pid=" << std::to_wstring(record.EventHeader.ProcessId);
                std::wcout << L" ttid=" << std::to_wstring(ttid);
                std::wcout << L" IoSize=" << std::to_wstring(io_size);
                auto pred = krabs::predicates::all_of({&predicates::events::sound_file_read});
                
                std::wcout << L" Satisfy Predicate=" << pred(record, trace_context);
                std::wcout << L"\n" << std::flush;
            }
        }
        catch (...) {
            std::wcout << L"some error occurred?";
            std::wcout << L"\n" << std::flush;
        }
        //}
    }
}

void print_rundown(const EVENT_RECORD &record, const krabs::trace_context &trace_context) {
    std::wcout << std::to_wstring(record.EventHeader.ProcessId) << L'\n' <<std::flush;
    std::wcout << L"hi\n" << std::flush;
    //krabs::schema schema(record, trace_context.schema_locator);
    //krabs::parser parser(schema);
    //auto opcode = schema.event_opcode();
    ////if (opcode == 106 || opcode == 107) {
    //    std::wstring filename = parser.parse<std::wstring>(L"Path");
    //    //if (filename.find(L"bk") != std::wstring::npos) {
    //        std::wcout << schema.task_name() << L"_" << schema.opcode_name();
    //        std::wcout << L" (" << schema.event_opcode() << L") ";
    //        std::wcout << L" filename=" << filename;
    //        std::cout << "\n" << std::flush;
    //    //}
    ////}
}

bool StartETW(void) {
    using namespace boost::sml;
    
    fsm::controller<> sm{};

    krabs::kernel_trace trace(L"kernel_trace");
    krabs::kernel::process_provider process_provider;
    krabs::kernel::file_init_io_provider fiio_provider;
    krabs::kernel::image_load_provider il_provider;

    il_provider.add_on_event_callback([&sm](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
        krabs::schema schema(record, trace_context.schema_locator);
        krabs::parser parser(schema);

        bool is_target = (*static_cast<krabs::predicates::details::predicate_base*>(
                                &predicates::filters::accepted_image_loads))(record, trace_context);
        if (is_target) {
            sm.handle_trace_event(record, trace_context);
        }
    });
    il_provider.add_on_event_callback([](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
    if ((record.EventHeader.EventDescriptor.Opcode == 3) ||
        (record.EventHeader.EventDescriptor.Opcode == 2) ||
        (record.EventHeader.EventDescriptor.Opcode == 10)) {  // ImageRundown
            krabs::schema schema(record, trace_context.schema_locator);
            krabs::parser parser(schema);
            std::wstring filename = parser.parse<std::wstring>(L"FileName");
            std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");

            if (pid == 19512) {
                std::wcout << L" (" << schema.event_opcode() << L") " << schema.task_name() << L"_"
                           << schema.opcode_name() << L" FileName=" << filename << L" PID=" << pid << L'\n'
                           << std::flush;
            }
    }
    });


    krabs::event_filter process_filter = predicates::filters::make_process_filter();
    krabs::event_filter fiio_filter = predicates::filters::make_fiio_filter();
    krabs::event_filter il_filter = predicates::filters::make_image_filter();

    auto dispatch = [&sm](const EVENT_RECORD &record, const krabs::trace_context &trace_context){
        try {
            sm.handle_trace_event(record, trace_context);
        }
        catch (const std::exception& exc) {
            std::cerr << exc.what();
            throw std::runtime_error("dispatch error");
        }
    };

    auto dummy = [](const EVENT_RECORD &record, const krabs::trace_context &trace_context) {
        int x;
        (void*)x;
    };


    process_filter.add_on_event_callback(dispatch);
    fiio_filter.add_on_event_callback(dispatch);

    //il_provider.add_on_event_callback([&sm](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
    //    bool is_target = (*static_cast<krabs::predicates::details::predicate_base*>(
    //                            &predicates::events::loaded_at_trace_start::wininet_image))(record, trace_context);
    //    if (is_target) {
    //        sm.handle_trace_event(record, trace_context);
    //    }
    //});
    il_filter.add_on_event_callback(dummy);


    process_provider.add_filter(process_filter);
    fiio_provider.add_filter(fiio_filter);
    il_provider.add_filter(il_filter);


    trace.enable(process_provider);
    trace.enable(fiio_provider);
    trace.enable(il_provider);

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