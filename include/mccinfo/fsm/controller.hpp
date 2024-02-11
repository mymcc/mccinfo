#pragma once
//#define BOOST_SML_CREATE_DEFAULT_CONSTRUCTIBLE_DEPS
#define MCCFSM_INLINE \
    inline constinit auto

#define MCCFSM_STATIC \
    static constexpr auto

#include "mccinfo/fsm/machines/mcc.hpp"
#include "mccinfo/fsm/machines/user.hpp"
#include "mccinfo/fsm/machines/game_id.hpp"

#include "edges/edges.hpp"
#include <iostream>
#include <string>
#include <ostream> 

namespace mccinfo {
namespace fsm {




inline events::event_t get_game_from_path(const std::wstring &file) {
    if (file.find(L"Halo1") != std::wstring::npos)
        return events::haloce_found{};
    else if (file.find(L"Halo2") != std::wstring::npos)
        return events::halo2_found{};
    else if (file.find(L"Halo2A") != std::wstring::npos)
        return events::halo2a_found{};
    else if (file.find(L"Halo3") != std::wstring::npos)
        return events::halo3_found{};
    else if (file.find(L"Halo3ODST") != std::wstring::npos)
        return events::halo3odst_found{};
    else if (file.find(L"Halo4") != std::wstring::npos)
        return events::halo4_found{};
    else if (file.find(L"HaloReach") != std::wstring::npos)
        return events::haloreach_found{};
    else
        throw std::runtime_error("get_game_from_path(): game not found");
}

inline bool file_has_open_handle(const std::wstring &file) {
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

inline std::optional<events::event_t> identify_game() {
    std::wstring prefix(L"F:\\SteamLibrary\\steamapps\\common\\Halo The Master Chief "
                        "Collection\\");

    std::wstring temp_prefix(L"C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Config\\");
    // halo1
    {
        std::wstring path = prefix + L"halo1\\sound\\pc\\sounds_stream.fsb";
        if (file_has_open_handle(path)) {
            return events::haloce_found{};
        }
    }

    // halo 2
    {
        // need to discern between h2 and h2a
        std::wstring pref_path = temp_prefix + L"Halo2\\preferences.dat";

        // halo2 classic mp/or h2/h2a campaign
        if (file_has_open_handle(pref_path)) {
            return events::halo2_found{}; 
        }

        // h2a mp
        std::wstring map_path = prefix + L"groundhog\\maps\\shared.map";
        if (file_has_open_handle(map_path)) {
            return events::halo2a_found{};
        }
    }

    // halo 3
    {
        std::wstring path = prefix + L"halo3\\maps\\shared.map";
        if (file_has_open_handle(path)) {
            return events::halo3_found{};
        }
    }

    // halo 4
    {
        std::wstring path = prefix + L"halo4\\maps\\shared.map";
        if (file_has_open_handle(path)) {
            return events::halo4_found{};
        }
    }

    // halo reach
    {
        std::wstring path = prefix + L"haloreach\\maps\\shared.map";
        if (file_has_open_handle(path)) {
            return events::haloreach_found{};
        }
    }

    return std::nullopt;
}

inline void print_trace_event(std::wostringstream& woss, const EVENT_RECORD &record,
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

namespace details {

class filtering_context {
  public:

    bool should_handle_trace_event(const EVENT_RECORD &record,
                                   const krabs::trace_context &trace_context) {
        bool is_target = (*static_cast<krabs::predicates::details::predicate_base *>(
            &predicates::filters::accepted_image_loads))(record, trace_context);

        if ((mcc_pid == UINT32_MAX) || 
            (mcc_pid == record.EventHeader.ProcessId) || 
            (is_target)) {

            if (is_target) {
                krabs::schema schema(record, trace_context.schema_locator);
                krabs::parser parser(schema);
                std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");
                if ((pid != mcc_pid) || (pid == UINT32_MAX) || (pid == 0))
                    return false;
            }
            return true;
        }
        return false;
    }

    template <typename _Controller, typename _StateMachine>
    void mcc_sm_event_wrapper(_Controller& controller,
                              _StateMachine &mcc_sm,
                              const EVENT_RECORD &record, 
                              const krabs::trace_context &trace_context) {
        bool current_is_off = false;
        if (!mcc_on) {
            current_is_off = mcc_sm.is(boost::sml::state<states::off>);
        }

        controller.handle_trace_event_impl(mcc_sm, record, trace_context);

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
        } else if (mcc_on && mcc_sm.is(boost::sml::state<states::off>)) {
            mcc_pid = UINT32_MAX;
            mcc_on = false;
            done_identification = false;
        }
    }

    template <typename _Controller, typename _StateMachineUser, typename _StateMachineGameId>
    void user_sm_event_wrapper(_Controller &controller, 
                               _StateMachineUser &user_sm,
                               _StateMachineGameId &game_id_sm,
                              const EVENT_RECORD &record,
                              const krabs::trace_context &trace_context) {
        controller.handle_trace_event_impl(user_sm, record, trace_context);

        if (user_sm.is(boost::sml::state<states::identifying_session>) && (!done_identification)) {
            using namespace constants::background_videos;

            auto video_basenames = menu::get_w(menu::video_keys::ALL);

            bool switch_to_in_match = true;
            for (const auto &wstr : video_basenames) {
                std::wstring target(L"F:\\SteamLibrary\\steamapps\\common\\Halo The Master Chief "
                                    "Collection\\mcc\\Content\\Movies\\");
                target += wstr;

                auto open = file_has_open_handle(target);

                if (open)
                    switch_to_in_match = false;
            }

            if (switch_to_in_match) {

                auto game_event = identify_game();

                if (game_event.has_value()) {
                    std::wcout << L"Sending Artificial match_found Event: " << L"\n";
                    user_sm.process_event(events::match_found{});
                    std::wcout << L"Sending Artificial game_event Event: " << L"\n";
                    std::visit([&](auto &&evt) { game_id_sm.process_event(evt); },
                               game_event.value());
                } else {
                    std::wcout << L"Sending Artificial launch_identified Event: " << L"\n";
                    user_sm.process_event(events::launch_identified{});
                }

            } else {
                std::wcout << L"Sending Artificial in_menus_identified Event: " << L"\n";
                user_sm.process_event(events::in_menus_identified{});
            }

            done_identification = true;
        }
    }

    uint32_t get_mcc_pid() const {
        return mcc_pid;
    }

    bool has_performed_identification() const {
        return done_identification;
    } 

  private:
    uint32_t mcc_pid = UINT32_MAX;
    bool mcc_on = false;
    bool done_identification = false;
};

}

template <class = class Dummy> class controller {
    friend class details::filtering_context;

  public:
    controller(callback_table& cbtable)
        : mcc_sm{cbtable},
        user_sm{cbtable},
        game_id_sm{cbtable}
    {};

    void handle_trace_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context) {
        utility::atomic_guard lk(lock);

        if (fc.should_handle_trace_event(record, trace_context)) {
            fc.mcc_sm_event_wrapper(*this, mcc_sm, record, trace_context);
            fc.user_sm_event_wrapper(*this, user_sm, game_id_sm, record, trace_context);
            handle_trace_event_impl<decltype(game_id_sm)>(game_id_sm, record, trace_context);
        }

    }
  private:
    template <typename _StateMachine>
    void handle_trace_event_impl(_StateMachine &sm, const EVENT_RECORD &record,
                                 const krabs::trace_context &trace_context) {

        std::wostringstream woss;
        print_trace_event(woss, record, trace_context);

        auto visit = [&](auto state) {
            states::BonusStateVisitor<_StateMachine> visitor(sm, record, trace_context, sc, woss);
            sm.visit_current_states(visitor);
        };
        sm.visit_current_states(visit);

        bool state_change = false;
        auto _evts = sc.pop_event_from_queue();
        while (_evts.has_value()) {
            std::visit(
                [&](auto &arg) {
                    auto ws = utility::ConvertBytesToWString(
                        std::string(utility::type_hash<decltype(arg)>::name));
                    if (ws.has_value())
                        woss << L"Sending Event: " << ws.value() << L"\n";
                    state_change = true;
                    sm.process_event(arg);
                },
                _evts.value());
            _evts = sc.pop_event_from_queue();
        }

        auto visit2 = [&](auto state) {
            states::StatePrinter<_StateMachine> visitor(sm, woss);
            sm.visit_current_states(visitor);
        };
        sm.visit_current_states(visit2);

        if (log_full || state_change)
            std::wcout << woss.str() << std::flush;
    }
  private:
    utility::atomic_mutex lock;

    states::state_context sc{};
    details::filtering_context fc{};
    boost::sml::sm<machines::mcc> mcc_sm;
    boost::sml::sm<machines::user> user_sm;
    boost::sml::sm<machines::game_id> game_id_sm;

  private: // filtering
    bool log_full = false;
    //bool log_full = true;
};
} // namespace fsm
} // namespace mccinfo