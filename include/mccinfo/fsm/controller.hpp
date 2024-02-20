#pragma once
//#define BOOST_SML_CREATE_DEFAULT_CONSTRUCTIBLE_DEPS
#define MCCFSM_INLINE \
    inline constinit auto

#define MCCFSM_STATIC \
    static constexpr auto

#include "mccinfo/fsm/autosave_client.hpp"
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
        auto bytes = utility::ConvertWStringToBytes(file);
        if (bytes.has_value()) {
            MI_CORE_TRACE("Sharing violation, file is guaranteed to have an open handle restricting access: {0}", bytes.value());
        }
        else {
            MI_CORE_TRACE("Sharing violation, file is guaranteed to have an open handle restricting access: Error logging file");
        }
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
    woss << L"\t";
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


inline mccinfo::file_readers::theater_file_data ReadTheaterFile(const std::filesystem::path &theater_file,
                     mccinfo::file_readers::game_hint hint) {
    //theater_file_timestamp.str("");
    mccinfo::file_readers::theater_file_data file_data;
    switch (hint) {
    case mccinfo::file_readers::game_hint::HALO2A: {
        mccinfo::file_readers::halo2a_theater_file_reader reader;
        auto file_data_query = reader.Read(theater_file);
        if (file_data_query.has_value()) {
            file_data = file_data_query.value();
            //theater_file_timestamp << file_data.utc_timestamp_;
        }
        break;
    }
    case mccinfo::file_readers::game_hint::HALO3: {
        mccinfo::file_readers::halo3_theater_file_reader reader;
        auto file_data_query = reader.Read(theater_file);
        if (file_data_query.has_value()) {
            file_data = file_data_query.value();
            //theater_file_timestamp << file_data.utc_timestamp_;
        }
        break;
    }
    case mccinfo::file_readers::game_hint::HALOREACH: {
        mccinfo::file_readers::haloreach_theater_file_reader reader;
        auto file_data_query = reader.Read(theater_file);
        if (file_data_query.has_value()) {
            file_data = file_data_query.value();
            //theater_file_timestamp << file_data.utc_timestamp_;
        }
        break;
    }
    case mccinfo::file_readers::game_hint::HALO4: {
        mccinfo::file_readers::halo4_theater_file_reader reader;
        auto file_data_query = reader.Read(theater_file);
        if (file_data_query.has_value()) {
            file_data = file_data_query.value();
            //theater_file_timestamp << file_data.utc_timestamp_;
        }
        break;
    }
    default:
        break;
    }

    return file_data;
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
                    std::wcout << L"\t\tSending Artificial match_found Event: " << L"\n";
                    user_sm.process_event(events::match_found{});
                    std::wcout << L"\t\tSending Artificial game_event Event: " << L"\n";
                    std::visit([&](auto &&evt) { game_id_sm.process_event(evt); },
                               game_event.value());
                } else {
                    std::wcout << L"\t\tSending Artificial launch_identified Event: " << L"\n";
                    user_sm.process_event(events::launch_identified{});
                }

            } else {
                std::wcout << L"\t\tSending Artificial in_menus_identified Event: " << L"\n";
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

struct map_info {
    std::string map;
};


}

struct extended_match_info {
    std::optional<std::filesystem::path> base_map_;
    std::optional<mccinfo::file_readers::theater_file_data> theater_file_data_;
    std::optional<mccinfo::file_readers::game_hint> game_hint_;
};

template <class = class Dummy> class controller {
    friend class details::filtering_context;

  public:
    controller(callback_table& cbtable)
        : mcc_sm{cbtable},
        user_sm{cbtable},
        game_id_sm{cbtable}, 
        //autosave_client_("", "", "HoboCopy.exe")
        autosave_client_("", "", "TScopy_x64.exe")
    {
        MI_CORE_TRACE("Constructing fsm controller ...");


        find_mcc_installations();



        autosave_client_.set_on_copy_start([&](const std::filesystem::path &path) { 
            for (const auto &file : std::filesystem::directory_iterator(path)) {
                if (std::filesystem::is_regular_file(file.path())) {
                    DeleteFile(file.path().generic_wstring().c_str());
                }
            }
        });

        MI_CORE_TRACE("Setting autosave destination to: .\\mccinfo_cache\\autosave");

        autosave_client_.set_copy_dst(".\\mccinfo_cache\\autosave");

        MI_CORE_TRACE("Starting autosave client ...");
        autosave_client_.start();
    };

    void handle_trace_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context) {
        utility::atomic_guard lk(lock);

        bool before_was_not_loading_in = !user_sm.is(boost::sml::state<states::loading_in>);
        bool before_was_not_in_game = !user_sm.is(boost::sml::state<states::in_game>);

        if (fc.should_handle_trace_event(record, trace_context)) {
            fc.mcc_sm_event_wrapper(*this, mcc_sm, record, trace_context);
            fc.user_sm_event_wrapper(*this, user_sm, game_id_sm, record, trace_context);
            handle_trace_event_impl<decltype(game_id_sm)>(game_id_sm, record, trace_context);

            if (user_sm.is(boost::sml::state<states::loading_in>) && before_was_not_loading_in) {
                should_id_map = true;
            }

            if (should_id_map) {
                // make predicate event more clear (specific map pred)
                if (predicates::events::map_file_created(record, trace_context)) {
                    krabs::schema schema(record, trace_context.schema_locator);
                    krabs::parser parser(schema);
                    std::wstring filename = parser.parse<std::wstring>(L"OpenPath");
                    auto bytes = utility::ConvertWStringToBytes(filename);
                    if (bytes.has_value()) {
                        mi.map = bytes.value();
                        emi_.base_map_ = bytes.value();
                    }
                    should_id_map = false;
                }   
            }

            if (user_sm.is(boost::sml::state<states::in_menus>)) {
                mi.map = "";
                emi_.base_map_ = std::nullopt;
                emi_.theater_file_data_ = std::nullopt;
                emi_.game_hint_ = std::nullopt;
            }

            if (user_sm.is(boost::sml::state<states::in_game>) && before_was_not_in_game) {
                should_save_autosave = true;
            }

            if (should_save_autosave) {
                std::string temp = "C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\";
                std::wstring game = L"";
                mccinfo::file_readers::game_hint hint = mccinfo::file_readers::game_hint::HALO3;

                assert(!game_id_sm.is(boost::sml::state<states::none>));

                if (game_id_sm.is(boost::sml::state<states::haloce>)) {
                    temp += "Halo1";
                    game = L"Halo1";
                    hint = mccinfo::file_readers::game_hint::HALO1;
                    //MessageBox(NULL, L"HALO1", L"", MB_OK);

                } else if (game_id_sm.is(boost::sml::state<states::halo2>)) {
                    temp += "Halo2";
                    game = L"Halo2";

                    hint = mccinfo::file_readers::game_hint::HALO2;
                    //MessageBox(NULL, L"HALO2", L"", MB_OK);


                } else if (game_id_sm.is(boost::sml::state<states::halo3>)) {
                    temp += "Halo3";
                    game = L"Halo3";

                    hint = mccinfo::file_readers::game_hint::HALO3;
                    //MessageBox(NULL, L"HALO3", L"", MB_OK);

                } else if (game_id_sm.is(boost::sml::state<states::halo3odst>)) {
                    temp += "Halo3ODST";
                    game = L"Halo3ODST";

                    hint = mccinfo::file_readers::game_hint::HALO3;
                    //MessageBox(NULL, L"HALO3ODST", L"", MB_OK);

                } else if (game_id_sm.is(boost::sml::state<states::haloreach>)) {
                    temp += "HaloReach";
                    game = L"HaloReach";

                    hint = mccinfo::file_readers::game_hint::HALOREACH;
                    //MessageBox(NULL, L"HALOREACH", L"", MB_OK);

                } else if (game_id_sm.is(boost::sml::state<states::halo4>)) {
                    temp += "Halo4";
                    game = L"Halo4";
                    hint = mccinfo::file_readers::game_hint::HALO4;
                    //MessageBox(NULL, L"HALO4", L"", MB_OK);

                } else if (game_id_sm.is(boost::sml::state<states::halo2a>)) {
                    temp += "Halo2A";
                    game = L"Halo2A";
                    hint = mccinfo::file_readers::game_hint::HALO2A;
                    //MessageBox(NULL, L"HALO2A", L"", MB_OK);
                }

                temp += "\\autosave";
                emi_.game_hint_ = hint;

                autosave_client_.set_copy_src(temp);
                autosave_client_.set_flatten_on_write(true);

                autosave_client_.set_on_complete([this, hint, game](const std::filesystem::path &src, const
                                                                  std::filesystem::path &path) {
                    mccinfo::file_readers::game_hint hint_ = hint;
                    std::wstring new_path = path.generic_wstring() + L"\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\";
                    new_path += game;
                    new_path += L"\\autosave";
                    std::filesystem::path new_path_w = new_path;
                    if (std::filesystem::exists(new_path_w) && std::filesystem::is_directory(new_path_w)) {
                        for (const auto &entry : std::filesystem::directory_iterator(new_path_w)) {
                            const auto &path2 = entry.path();
                            
                            if (std::filesystem::is_regular_file(path2)) {
                                if ((path2.extension().generic_string() == ".temp") ||
                                    (path2.extension().generic_string() == ".film")) {
                                    //MessageBox(NULL, path2.generic_wstring().c_str(), L"", MB_OK);
                                    this->emi_.theater_file_data_ = ReadTheaterFile(std::filesystem::canonical(path2), hint_);
                                }
                            }
                        }
                    }

                    std::wstring del_path = (path.generic_wstring() + L"\\Users");
                    RemoveDirectory(del_path.c_str());
                    for (const auto &file : std::filesystem::directory_iterator(src)) {
                        if (std::filesystem::is_regular_file(file.path())) {
                            DeleteFile(file.path().generic_wstring().c_str());
                        }
                    }
                });

                // 5 second delay should be sufficient for h2a/h4
                if ((hint == mccinfo::file_readers::game_hint::HALO2A) ||
                    (hint == mccinfo::file_readers::game_hint::HALO4)) {
                    autosave_client_.request_copy(5000);
                }
                autosave_client_.request_copy(1000);
                should_save_autosave = false;
            }



        }
    }
    std::string get_map_info() const {
        return mi.map;
    }
    mccinfo::file_readers::theater_file_data get_theater_file_data() const {
        return file_data;
    }
    std::vector<std::optional<mccinfo::query::MCCInstallInfo>> get_install_info() const {
        std::vector<std::optional<mccinfo::query::MCCInstallInfo>> installations;
        
        installations.push_back(steam_install_);
        installations.push_back(msstore_install_);
        
        return installations;
    }

    const extended_match_info& get_extended_match_info() const {
        return emi_;
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
                        woss << L"\t\tSending Event: " << ws.value() << L"\n";
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

        if (log_full || state_change) {
            auto bytes = utility::ConvertWStringToBytes(woss.str());
            if (bytes.has_value())
                MI_CORE_TRACE("Handling kernel event:\n{0}", bytes.value());
            else
            {
                MI_CORE_WARN("Handling kernel event ... Error converting logging trace event");

            }
        }
    }

    void find_mcc_installations() {
        MI_CORE_TRACE("Looking for MCC Installations ...");

        auto sii = mccinfo::query::LookForSteamInstallInfo();
        if (sii.has_value()) {
            std::wostringstream woss;
            woss << sii.value();
            auto bytes = utility::ConvertWStringToBytes(woss.str());
            // assert here or provide an overload for string
            MI_CORE_INFO("Steam Install Found:\n{0}", bytes.value());
            steam_install_ = sii.value();
        }

        auto msii = mccinfo::query::LookForMicrosoftStoreInstallInfo();
        if (msii.has_value()) {
            std::wostringstream woss;
            woss << msii.value();
            auto bytes = utility::ConvertWStringToBytes(woss.str());
            MI_CORE_INFO("MSStore Install Found:\n{0}", bytes.value());
            msstore_install_ = msii.value();
        }
    }
  private:
    utility::atomic_mutex lock;

    states::state_context sc{};
    details::filtering_context fc{};
    boost::sml::sm<machines::mcc> mcc_sm;
    boost::sml::sm<machines::user> user_sm;
    boost::sml::sm<machines::game_id> game_id_sm;
    mccinfo::file_readers::theater_file_data file_data;
    mccinfo::fsm::autosave_client autosave_client_;

    extended_match_info emi_;

    details::map_info mi;

  private:
    std::optional<query::MCCInstallInfo> steam_install_ = std::nullopt;
    std::optional<query::MCCInstallInfo> msstore_install_ = std::nullopt;


    bool should_id_map = false;
    bool should_save_autosave = false;

  private: // filtering
    bool log_full = false;
    //bool log_full = true;
};
} // namespace fsm
} // namespace mccinfo