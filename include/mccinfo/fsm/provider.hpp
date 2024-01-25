#pragma once

#include "mccinfo/query.hpp"
#include "mccinfo/fsm/controller.hpp"
#include "mccinfo/fsm/predicates.hpp"
#include "lockfree/lockfree.hpp"
#include <iostream>
#include <thread>

namespace mccinfo {
namespace fsm {

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

namespace details {

inline bool concrete_image_filter(const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
    return (*static_cast<krabs::predicates::details::predicate_base *>(
        &predicates::filters::accepted_image_loads))(record, trace_context);
}

} // details

class event_provider {
  public:
    event_provider() : 
        trace_(L"mccinfo_kernel_trace") {

    }

    // TODO Figure a way around the static stuff here (another state machine)
    template <typename T>
    void enable_dispatch_to(T* sm) {
        static krabs::event_filter process_filter = predicates::filters::make_process_filter();
        static krabs::event_filter fiio_filter = predicates::filters::make_fiio_filter();
        static krabs::event_filter il_filter = predicates::filters::make_dummy_image_filter();

        static auto dispatch_event = 
            [=] (const EVENT_RECORD &record, const krabs::trace_context &trace_context) {
                try {
                    sm->handle_trace_event(record, trace_context);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
            };

        static auto dispatch_image_event =
            [=](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
                try {
                    if (details::concrete_image_filter(record, trace_context)) {
                        sm->handle_trace_event(record, trace_context);
                    }
                }
                catch (std::exception& e) {
                        std::cerr << e.what() << std::endl;
                }
            };

        il_provider_.add_on_event_callback(dispatch_image_event);

        static auto dummy = 
            [](const EVENT_RECORD &record, const krabs::trace_context &trace_context) {
                int x;
                (void*)x;
            };

        process_filter.add_on_event_callback(dispatch_event);
        fiio_filter.add_on_event_callback(dispatch_event);
        il_filter.add_on_event_callback(dummy);

        process_provider_.add_filter(process_filter);
        fiio_provider_.add_filter(fiio_filter);
        il_provider_.add_filter(il_filter);

        trace_.enable(process_provider_);
        trace_.enable(fiio_provider_);
        trace_.enable(il_provider_);

    }
    void start() {
        trace_.start();
    }
    void stop() {
        trace_.stop();
    }

  private:
    void set_filters() {}

  private:
    krabs::kernel_trace trace_;
    krabs::kernel::process_provider process_provider_;
    krabs::kernel::file_init_io_provider fiio_provider_;
    krabs::kernel::image_load_provider il_provider_;
};

} // namespace fsm
} // namespace mccinfo