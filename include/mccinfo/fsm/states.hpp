#pragma once

#include <fsm/fsm.hpp>
#include "..\utility.hpp"
#include "static_predicates.hpp"

namespace mccinfo {
namespace fsm2 {
namespace states {
struct mcc_initial;
struct mcc_off;
struct mcc_loading;
struct mcc_on;

struct launch_initial;
struct launch_start;
struct launch_with_eac;
struct launch_no_eac;

struct launch_initial {
    using transitions = ctfsm::type_map<std::pair<events::launch_start, launch_start>>;
    static constexpr std::string_view id{"LAUNCH INITIAL"};
};

struct launch_start {
    using transitions = ctfsm::type_map<std::pair<events::launch_reset, launch_initial>,
                                        std::pair<events::eac, launch_with_eac>,
                                        std::pair<events::no_eac, launch_no_eac>>;

    static constexpr std::string_view id{"LAUNCH START"};
};

struct launch_with_eac {
    using transitions = ctfsm::type_map<std::pair<events::launch_reset, launch_initial>>;

    static constexpr std::string_view id{"LAUNCH WITH EAC"};
};

struct launch_no_eac {
    using transitions = ctfsm::type_map<std::pair<events::launch_reset, launch_initial>>;

    static constexpr std::string_view id{"LAUNCH WITH NO EAC"};
};

static utility::atomic_mutex s_Lock;

template <typename Derived> struct trace_event_handler {
    template <typename fsm_type>
    void handle_trace_event(fsm_type *fsm, const EVENT_RECORD &record,
                            const krabs::trace_context &trace_context) {
        utility::atomic_guard lk(s_Lock);

        if (Derived::seq.try_advance(record, trace_context)) {
            std::cout << "handling trace event" << std::endl;
            krabs::schema schema(record, trace_context.schema_locator);
            krabs::parser parser(schema);

            if (schema.event_opcode() != 11) { // Prevent Process_Terminate (Event Version(2))
                
            }
        }
        if (Derived::seq.is_complete()) {
            std::cout << "should transition" << std::endl;
            fsm->handle_event(Derived::seq.consume_next_event());
        }
    }
};

struct mcc_initial : public trace_event_handler<mcc_initial> {
    using transitions = ctfsm::type_map<std::pair<events::start, mcc_off>>;
    static constexpr std::string_view id{"INITIAL"};
    static constexpr predicate_sequence<events::start, 0> seq{}; // meant to be unused

    void on_enter() {
        std::cout << "INITIAL entered" << std::endl;
    }
};

struct mcc_off : public trace_event_handler<mcc_off> {
    using transitions = ctfsm::type_map<std::pair<events::reset, mcc_initial>,
                                        std::pair<events::launcher_start, mcc_loading>>;

    static constexpr std::string_view id{"MCC:OFF"};

    static constexpr predicate_sequence<events::launcher_start, 2> seq{
        &predicates::launcher_start_pred, &predicates::eac_start_pred};

    void on_enter() {
        std::cout << "mcc_off entered" << std::endl;
    }
};

struct mcc_loading : public trace_event_handler<mcc_loading> {
    using transitions = ctfsm::type_map<std::pair<events::reset, mcc_initial>,
                                        std::pair<events::launcher_failed, mcc_off>,
                                        std::pair<events::launcher_end, mcc_on>>;

    static constexpr std::string_view id{"MCC:LOADING"};
    // static constinit ctfsm::fsm<fsm::states::launch_initial> fsm;

    static constexpr auto seq = make_predicate_sequence<events::launcher_end>(&predicates::launcher_end_pred);

    void on_enter() {
        std::cout << "mcc_loading entered" << std::endl;
    }
};
struct mcc_on : public trace_event_handler<mcc_on> {
    using transitions = ctfsm::type_map<std::pair<events::reset, mcc_initial>,
                                        std::pair<events::process_lost, mcc_off>>;

    static constexpr std::string_view id{"MCC:ON"};
    static constexpr predicate_sequence<events::process_lost, 1> seq{
        &predicates::mcc_process_lossed};

    void on_enter() {
        std::cout << "mcc_on entered" << std::endl;
    }
};

} // namespace states
} // namespace fsm2
} // namespace mccinfo