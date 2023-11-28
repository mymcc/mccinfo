#pragma once
//#define BOOST_SML_CREATE_DEFAULT_CONSTRUCTIBLE_DEPS
#define MCCFSM_INLINE \
    inline constinit auto

#define MCCFSM_STATIC \
    static constexpr auto

#include "machines/mcc.hpp"
#include "machines/launch.hpp"

#include "edges/edges.hpp"
#include <iostream>
#include <string>

namespace mccinfo {
namespace fsm {

void print_trace_event(const EVENT_RECORD &record,
                                               const krabs::trace_context &trace_context) {
    krabs::schema schema(record, trace_context.schema_locator);
    krabs::parser parser(schema);

    if (schema.event_opcode() != 11) { // Prevent Process_Terminate (Event Version(2))
        std::string imagefilename = parser.parse<std::string>(L"ImageFileName");

        std::wcout << schema.task_name() << L"_" << schema.opcode_name();
        std::wcout << L" (" << schema.event_opcode() << L") ";
        std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");
        std::wcout << L" ProcessId=" << pid;
        std::cout << " ImageFileName=" << imagefilename;
        
        std::wcout << std::endl;
    }
}

constexpr auto can_find_launcher = []() {
    auto can_find_launcher_impl = []() {
        auto mcc_launcher_pid = utility::GetProcessIDFromName(std::wstring(L"mcclauncher.exe"));
        if (mcc_launcher_pid.has_value()) {
            return mcc_launcher_pid;
        } else {
            std::optional<size_t> ret = std::nullopt;
            return ret;
        }
    };
    return can_find_launcher_impl().has_value();
};

template <class = class Dummy> class controller {
  public:
    void initialize() {
        if (!initialized) {
            if (machines::can_find_mcc() && !can_find_launcher()) {
                mcc_sm.process_event(events::mcc_found{});
            }
        }
    }
    void handle_trace_event(const EVENT_RECORD &record,
                            const krabs::trace_context &trace_context) {
        utility::atomic_guard lk(lock);

        std::cout << "=============================================================\n";
        print_trace_event(record, trace_context);

        auto visit = [&](auto state) {
            states::BonusStateVisitor<decltype(mcc_sm)> visitor(mcc_sm, record, trace_context, sc);
            mcc_sm.visit_current_states(visitor);
        };
        mcc_sm.visit_current_states(visit);
        
        auto _evts = sc.pop_event_from_queue();
        while (_evts.has_value()) {
            std::visit(
                [&](auto &arg) {
                    std::cout << "Sending Event: " << utility::type_hash<decltype(arg)>::name << "\n";
                    mcc_sm.process_event(arg);
                    std::cout << "" << std::flush;
                }, _evts.value());
            _evts = sc.pop_event_from_queue();
        }

        auto visit2 = [&](auto state) {
            states::StatePrinter<decltype(mcc_sm)> visitor(mcc_sm);
            mcc_sm.visit_current_states(visitor);
        };
        mcc_sm.visit_current_states(visit2);

    }
  private:
    utility::atomic_mutex lock;
    //boost::sml::sm<machines::mcc, machines::launch> sm;

    boost::sml::sm<machines::mcc> mcc_sm;
    states::state_context sc{};
    bool initialized = false;
    //boost::sml::sm<machines::launch> launch_sm;
};
} // namespace fsm
} // namespace mccinfo