#pragma once

#include "mccinfo/query.hpp"
#include "mccinfo/fsm/mccfsm.hpp"
#include "mccinfo/fsm/predicates.hpp"
#include "lockfree/lockfree.hpp"
#include <iostream>
#include <thread>
#include <condition_variable>

namespace mccinfo {
namespace fsm {
namespace provider {

class event_dispatcher {
  public:
    template <typename T> void start(T& sm_controller) {
        auto dispatch = [&] {
            while (true) {
                std::lock_guard lk(mut_);
                
                if (stop_)
                    break;

                EVENT_RECORD record;
                bool read_success = queue_.Pop(record);
                if (read_success) {
                    try {
                        sm_controller.handle_trace_event(record, *trace_context_);
                    }
                    catch (const std::exception& exc) {
                        std::cerr << exc.what();
                        throw std::runtime_error("dispatch error");
                    }
                } else {
                    std::this_thread::yield();
                }
            }
        };

        dispatch_thread_ = std::thread(dispatch);
    }

    void enqueue(const EVENT_RECORD &record) {
        queue_.Push(record);
    }

    void set_trace_context(const krabs::trace_context* trace_context) {
        trace_context_ = const_cast<krabs::trace_context*>(trace_context);
    }
    
    void stop() {
        {
            std::lock_guard lk(mut_);
            stop_ = true;
        }
        dispatch_thread_.join();
    }

  private:
    bool stop_ = false;
    std::mutex mut_;
    std::condition_variable cv_;
    lockfree::spsc::Queue<EVENT_RECORD, 1000> queue_;
    krabs::trace_context* trace_context_;
    std::thread dispatch_thread_;
};

bool StartETW(void) {
    using namespace boost::sml;
    
    fsm::controller<> sm{};
    event_dispatcher dispatcher{};

    krabs::kernel_trace trace(L"kernel_trace");
    
    krabs::kernel::process_provider process_provider;
    krabs::kernel::file_init_io_provider fiio_provider;
    krabs::kernel::image_load_provider il_provider;

    il_provider.add_on_event_callback([&sm, &dispatcher](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
        krabs::schema schema(record, trace_context.schema_locator);
        krabs::parser parser(schema);
        
        bool is_target = (*static_cast<krabs::predicates::details::predicate_base*>(
                                &predicates::filters::accepted_image_loads))(record, trace_context);
        if (is_target) {
            //dispatcher.set_trace_context(&trace_context);
            //dispatcher.enqueue(record);
            sm.handle_trace_event(record, trace_context);
        }
    });
    
    krabs::event_filter process_filter = predicates::filters::make_process_filter();
    krabs::event_filter fiio_filter = predicates::filters::make_fiio_filter();
    krabs::event_filter il_filter = predicates::filters::make_image_filter();

    auto dispatch = [&sm, &dispatcher](const EVENT_RECORD &record, const krabs::trace_context &trace_context){
        try {
            //dispatcher.set_trace_context(&trace_context);
            //dispatcher.enqueue(record);
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
    il_filter.add_on_event_callback(dummy);

    process_provider.add_filter(process_filter);
    fiio_provider.add_filter(fiio_filter);
    il_provider.add_filter(il_filter);

    trace.enable(process_provider);
    trace.enable(fiio_provider);
    trace.enable(il_provider);

    std::cout << " - starting trace" << std::endl;
    
    std::thread thread([&trace, &dispatcher, &sm]() {
        //dispatcher.start(sm);
        trace.start();
    });

    while (true) {
        std::string input;
        std::getline(std::cin, input);
        if (input == "q") {
            std::cout << std::endl << " - stopping trace" << std::endl;
            trace.stop();
            //dispatcher.stop();
            break;
        }
    }

    thread.join();

    return true;
}
} // namespace provider
} // namespace fsm
} // namespace mccinfo