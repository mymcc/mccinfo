#pragma once
//#define BOOST_SML_CREATE_DEFAULT_CONSTRUCTIBLE_DEPS
#define MCCFSM_INLINE \
    inline constinit auto

#define MCCFSM_STATIC \
    static constexpr auto

#include "mccinfo/fsm/machines/mcc.hpp"
#include "mccinfo/fsm/machines/user.hpp"

#include "edges/edges.hpp"
#include <iostream>
#include <string>
#include <ostream> 

namespace mccinfo {
namespace fsm {

bool file_has_open_handle(const std::wstring &file) {
    HANDLE hFile = CreateFileW(file.c_str(),   // name of the write
                       GENERIC_WRITE,          // open for writing
                       0,                      // *** do not share ***
                       NULL,                   // default security
                       OPEN_EXISTING,          // create new file only
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template

    if ((hFile == INVALID_HANDLE_VALUE) && (GetLastError() == ERROR_SHARING_VIOLATION)) {
        std::wcout << "sharing violation\n" << std::flush;
        return true;
    }
    else {
        CloseHandle(hFile);
        return false;
    }
}

void print_trace_event(std::wostringstream& woss, const EVENT_RECORD &record,
                                               const krabs::trace_context &trace_context) {
    woss << L"=============================================================\n";

    krabs::schema schema(record, trace_context.schema_locator);
    krabs::parser parser(schema);
    
    try {
        if (schema.event_opcode() != 11) { // Prevent Process_Terminate (Event Version(2))
            if (schema.event_opcode() != 64) {
                if (schema.event_opcode() != 67) {
                    if ((schema.event_opcode() == 3) && ((std::wstring(schema.task_name()).find(L"Process") == std::wstring::npos))) {

                        std::wstring imagefilename = parser.parse<std::wstring>(L"FileName");
                        std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");

                        woss << schema.task_name() << L"_" << schema.opcode_name();
                        woss << L" (" << schema.event_opcode() << L") ";
                        woss << L" ProcessId=" << pid;
                        woss << L" ImageFileName=" << imagefilename;
                        
                    } else {
                        std::string imagefilename = parser.parse<std::string>(L"ImageFileName");
                        std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");
                        woss << schema.task_name() << L"_" << schema.opcode_name();
                        woss << L" (" << schema.event_opcode() << L") ";
                        woss << L" ProcessId=" << pid;
                        auto ws = utility::ConvertBytesToWString(imagefilename);
                        if (ws.has_value())
                            woss << L" ImageFileName=" << ws.value();
                    }
                } else {
                    uint32_t ttid = parser.parse<uint32_t>(L"TTID");
                    uint32_t io_size = parser.parse<uint32_t>(L"IoSize");

                    woss << schema.task_name() << L"_" << schema.opcode_name();
                    woss << L" (" << schema.event_opcode() << L") ";
                    woss << L" pid=" << std::to_wstring(record.EventHeader.ProcessId);
                    woss << L" ttid=" << std::to_wstring(ttid);
                    woss << L" IoSize=" << std::to_wstring(io_size);
                }
            } else {
                std::wstring imagefilename = parser.parse<std::wstring>(L"OpenPath");
                woss << schema.task_name() << L"_" << schema.opcode_name();
                woss << L" (" << schema.event_opcode() << L") ";
                woss << " Path=" << imagefilename;
            }
            woss << std::endl;
        }
    }
    catch (const std::exception& exc) {
        std::cerr << exc.what();
        throw std::runtime_error("hi :)))))))))))");
    }
}


struct controller_context {


private:
    bool needs_id_ = false;
    size_t mcc_pid_ = SIZE_MAX;
    states::state_context sc_{};
};

template <class = class Dummy> class controller {
  public:
    void handle_trace_event(const EVENT_RECORD &record,
                            const krabs::trace_context &trace_context) {
        utility::atomic_guard lk(lock);

        bool is_target = (*static_cast<krabs::predicates::details::predicate_base*>(
                                &predicates::filters::accepted_image_loads))(record, trace_context);

        if ((mcc_pid == UINT32_MAX) || (mcc_pid == record.EventHeader.ProcessId) || (is_target)) {

            if (is_target) {
                krabs::schema schema(record, trace_context.schema_locator);
                krabs::parser parser(schema);
                std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");
                if ((pid != mcc_pid) || (pid == UINT32_MAX) || (pid == 0)) return;
            }

            handle_trace_event_impl<decltype(mcc_sm)>(mcc_sm, record, trace_context);
            handle_trace_event_impl<decltype(play_sm)>(play_sm, record, trace_context);
        }
    }
  private:
    template<typename _StateMachine>
    void handle_trace_event_impl(_StateMachine& sm, const EVENT_RECORD &record, const krabs::trace_context &trace_context) {

        std::wostringstream woss;
        print_trace_event(woss, record, trace_context);

        auto visit = [&](auto state) {
            states::BonusStateVisitor<_StateMachine> visitor(sm, record, trace_context, sc, woss);
            sm.visit_current_states(visitor);
        };
        sm.visit_current_states(visit);

        bool current_is_off = false;
        if (!mcc_on) {
            current_is_off = mcc_sm.is(boost::sml::state<states::off>);
        }
        bool state_change = false;

        auto _evts = sc.pop_event_from_queue();
        while (_evts.has_value()) {
            std::visit(
                [&](auto &arg) {
                    auto ws = utility::ConvertBytesToWString(std::string(utility::type_hash<decltype(arg)>::name));
                    if (ws.has_value())
                    woss << L"Sending Event: " << ws.value() << L"\n";
                    state_change = true;
                    sm.process_event(arg);
                }, _evts.value());
            _evts = sc.pop_event_from_queue();
        }

        auto visit2 = [&](auto state) {
            states::StatePrinter<_StateMachine> visitor(sm, woss);
            sm.visit_current_states(visitor);
        };
        sm.visit_current_states(visit2);

        if (!mcc_on && mcc_sm.is(boost::sml::state<states::on>)) {
            if (current_is_off) {
                krabs::schema schema(record, trace_context.schema_locator);
                krabs::parser parser(schema);
                std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");
                mcc_pid = pid;
            } else {
                mcc_pid = record.EventHeader.ProcessId;
            }
            mcc_on = true;
        }
        else if (mcc_on && mcc_sm.is(boost::sml::state<states::off>)) {
            mcc_pid = UINT32_MAX;
            mcc_on = false;
        }

        if (play_sm.is(boost::sml::state<states::identifying_session>) && (!done_identification)) {
            using namespace constants::background_videos;

            auto video_basenames = menu::get_w(menu::video_keys::ALL);

            bool switch_to_in_match = true;
            for (const auto &wstr : video_basenames) {
                std::wstring target(L"F:\\SteamLibrary\\steamapps\\common\\Halo The Master Chief "
                           "Collection\\mcc\\Content\\Movies\\");
                target += wstr;

                auto open = file_has_open_handle(target);

                //std::wcout << open << L" : " << target << L'\n' << std::flush;
                if (open)
                    switch_to_in_match = false;
            }

            if (switch_to_in_match) {
                std::wcout << L"Sending Artificial match_found Event: " << L"\n";
                sm.process_event(events::match_found{});
            } else {
                std::wcout << L"Sending Artificial in_menus_identified Event: " << L"\n";
                sm.process_event(events::in_menus_identified{});
            }

            done_identification = true;
        }

        if (log_full || state_change) std::wcout << woss.str() << std::flush;
    }
  private:
    utility::atomic_mutex lock;
    boost::sml::sm<machines::mcc> mcc_sm;
    boost::sml::sm<machines::user> play_sm;
    states::state_context sc{};
    uint32_t mcc_pid = UINT32_MAX;
    bool mcc_on = false;
    //bool log_full = false;
    bool log_full = true;

    bool done_identification = false;
};
} // namespace fsm
} // namespace mccinfo