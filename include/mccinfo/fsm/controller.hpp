#pragma once
//#define BOOST_SML_CREATE_DEFAULT_CONSTRUCTIBLE_DEPS
#define MCCFSM_INLINE \
    inline constinit auto

#define MCCFSM_STATIC \
    static constexpr auto

#include "mccinfo/fsm/autosave_client.hpp"
#include "mccinfo/fsm/machines/machines.hpp"

namespace mccinfo {
namespace fsm {
namespace details {

inline void collect_leftover_autosave_files(const std::filesystem::path& root,
                                            std::vector<std::filesystem::path>& film_files,
                                            std::vector<std::filesystem::path>& map_files,
                                            std::vector<std::filesystem::path>& game_files) {
    if (std::filesystem::is_directory(root)) {
        for (const auto &file : std::filesystem::directory_iterator(root)) {
            if (std::filesystem::is_regular_file(file.path())) {
                auto extension = file.path().extension();
                if (extension == ".film") {
                    film_files.push_back(file);
                } else if (extension == ".map") {
                    map_files.push_back(file);
                } else if (extension == ".game") {
                    game_files.push_back(file);
                }
            }
        }

        std::sort(map_files.begin(), map_files.end(), utility::by_last_file_write_time);
        std::sort(game_files.begin(), game_files.end(), utility::by_last_file_write_time);

        if (!map_files.empty())
            map_files.pop_back();

        if (!game_files.empty())
            game_files.pop_back();
    }
}

inline void move_leftover_autosave_files(const std::filesystem::path &root,
                                         std::vector<std::filesystem::path> &film_files,
                                         std::vector<std::filesystem::path> &map_files,
                                         std::vector<std::filesystem::path> &game_files) {

    for (const auto &file : film_files) {
        auto target = root / "Movie" / file.filename().generic_string();

        MI_CORE_TRACE("Copying extraneous autosave .FILM file:\n\tSubject: {0}\n\tTo: {1}",
                      file.generic_string().c_str(), target.generic_string().c_str());

        try {
            std::filesystem::copy_file(file, target);

            std::filesystem::remove(file);
        } catch (std::exception &e) {
            MI_CORE_ERROR("Filesystem Error\nException: {0}", e.what());
        }
    }
    for (const auto &file : map_files) {
        auto target = root / "Map" / file.filename().generic_string();

        MI_CORE_TRACE("Copying extraneous autosave .MAP file:\n\tSubject: {0}\n\tTo: {1}",
                      file.generic_string().c_str(), target.generic_string().c_str());

        try {
            std::filesystem::copy_file(file, target);

            std::filesystem::remove(file);
        } catch (std::exception &e) {
            MI_CORE_ERROR("Filesystem Error\nException: {0}", e.what());
        }
    }
    for (const auto &file : game_files) {
        auto target = root / "GameType" / file.filename().generic_string();

        MI_CORE_TRACE("Copying extraneous autosave .GAME file:\n\tSubject: {0}\n\tTo: {1}",
                      file.generic_string().c_str(), target.generic_string().c_str());

        try {
            std::filesystem::copy_file(file, target);

            std::filesystem::remove(file);
        } catch (std::exception &e) {
            MI_CORE_ERROR("Filesystem Error\nException: {0}", e.what());
        }
    }
}

inline void flush_leftover_autosave_files(const std::filesystem::path &src) {
    std::vector<std::filesystem::path> film_files;
    std::vector<std::filesystem::path> map_files;
    std::vector<std::filesystem::path> game_files;

    details::collect_leftover_autosave_files(src, film_files, map_files, game_files);

    auto temp_path = std::filesystem::path(
        utility::ConvertWStringToBytes(query::LookForMCCTempPath().value()).value());

    auto user_content =                                        
        temp_path /                                           
            "Temporary/UserContent" /                         
                src.parent_path().filename().generic_string();

    details::move_leftover_autosave_files(user_content, film_files, map_files, game_files);
}

class filtering_context {
  public:

    bool should_handle_trace_event(const EVENT_RECORD &record,
                                   const krabs::trace_context &trace_context) {
        if ((mcc_pid == std::numeric_limits<uint32_t>::max()) || 
            (mcc_pid == record.EventHeader.ProcessId)) {
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

                auto open = utility::FileHasOpenHandle(target);

                if (open)
                    switch_to_in_match = false;
            }

            if (switch_to_in_match) {

                auto game_hint = query::IdentifyCurrentGame(
                    L"F:\\SteamLibrary\\steamapps\\common\\Halo The Master Chief "
                    "Collection");

                if (game_hint.has_value()) {
                    std::wcout << L"\t\tSending Artificial match_found Event: " << L"\n";
                    user_sm.process_event(events::match_found{});
                    std::wcout << L"\t\tSending Artificial game_event Event: " << L"\n";
                    std::visit([&](auto &&evt) { game_id_sm.process_event(evt); },
                                events::GetGameEventFromHint(game_hint.value()));
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

  private:
    uint32_t mcc_pid = std::numeric_limits<uint32_t>::max();
    bool done_identification = false;
    bool mcc_on = false;
};

struct map_info {
    std::string map;
};


}

struct extended_match_info {

    std::optional<std::filesystem::path> base_map_;
    std::optional<std::filesystem::path> carnage_report_;
    std::optional<mccinfo::file_readers::theater_file_data> theater_file_data_;
    std::optional<mccinfo::game_hint> game_hint_;

    void reset() {
        base_map_ = std::nullopt;
        theater_file_data_ = std::nullopt;
        game_hint_ = std::nullopt;
        carnage_report_ = std::nullopt;
    }
};

template <class = class Dummy> class controller {
    friend class details::filtering_context;

  public:
    controller(callback_table& cbtable)
        : mcc_sm{cbtable},
        user_sm{cbtable},
        game_id_sm{cbtable},
        cb_table_{cbtable},
        autosave_client_("", "", "TScopy_x64.exe")
    {
        MI_CORE_TRACE("Constructing fsm controller ...");

        find_mcc_installations();
        add_match_data_collector_callbacks();
        emi_.reset();

        autosave_client_.set_on_copy_start(
            [&](const std::filesystem::path &src, const std::filesystem::path &dst) {
                details::flush_leftover_autosave_files(src);
        });

        MI_CORE_TRACE("Setting autosave destination to: .\\mccinfo_cache\\autosave");

        autosave_client_.set_copy_dst(".\\mccinfo_cache\\autosave");

        MI_CORE_TRACE("Starting autosave client ...");
        autosave_client_.start();
    };

    void handle_trace_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context) {
        utility::atomic_guard lk(lock);

        bool before_was_not_loading_in = !user_sm.is(boost::sml::state<states::loading_in>);
        bool before_was_not_loading_out = !user_sm.is(boost::sml::state<states::loading_out>);
        bool before_was_not_in_game = !user_sm.is(boost::sml::state<states::in_game>);

        if (fc.should_handle_trace_event(record, trace_context)) {
            fc.mcc_sm_event_wrapper(*this, mcc_sm, record, trace_context);
            fc.user_sm_event_wrapper(*this, user_sm, game_id_sm, record, trace_context);
            handle_trace_event_impl<decltype(game_id_sm)>(game_id_sm, record, trace_context);

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

            if (should_copy_cr) {
                // should really be on LOADING_OUT | STATE_EXIT
                if (!user_sm.is(boost::sml::state<states::loading_out>) &&
                    !user_sm.is(boost::sml::state<states::in_game>)) {

                    if (emi_.carnage_report_.has_value()) {
                        std::filesystem::path to = std::filesystem::path(".\\mccinfo_cache\\autosave") /
                                                    emi_.carnage_report_.value().filename();

                        auto temp = query::LookForMCCTempPath();
                        if (temp.has_value()) {
                            auto cr_parent = std::filesystem::path(temp.value()) / "Temporary";
                            for (const auto &file : std::filesystem::directory_iterator(cr_parent)) {
                                MI_CORE_TRACE("Comparing: {0} to {1}", 
                                    file.path().generic_string().c_str(),
                                    emi_.carnage_report_.value().generic_string().c_str());

                                if (std::filesystem::is_regular_file(file.path())) {
                                    if (file.path().filename() == emi_.carnage_report_.value().filename()) {
                                        MI_CORE_INFO("Copying carnage report: {0}", 
                                            file.path().generic_string().c_str());

                                        try {
                                            std::filesystem::copy_file(file.path(), to);
                                        } catch (std::exception &e) {
                                            MI_CORE_ERROR("Error copying file {0}\nException: {1}",
                                                          file.path().generic_string().c_str(),
                                                          e.what());
                                        }
                                        break;
                                    }
                                }
                            }

                        }
                    
                    }
                    // we didn't identify a cr on exit (likely firefight or campaign)
                    else 
                    {
                        MI_CORE_WARN("No carnage report identified on loading_out, searching MCC Temp Path for latest carnage report ...");

                        auto temp = query::LookForMCCTempPath();
                        if (temp.has_value()) {
                            auto cr_parent = std::filesystem::path(temp.value()) / "Temporary";

                        
                            std::filesystem::path latest_cr;
                            std::filesystem::file_time_type latest_cr_write =
                                std::filesystem::file_time_type::clock::time_point(std::chrono::seconds(0));

                            for (const auto &file : std::filesystem::directory_iterator(cr_parent)) {
                                if (std::filesystem::is_regular_file(file.path()) && file.path().has_extension()) {
                                    if (file.path().extension().generic_string() == ".xml") {

                                        bool newer = file.last_write_time() > latest_cr_write;

                                        MI_CORE_TRACE("Comparing write times: ({0} > {1}) ? => {2}",
                                                      file.path().generic_string().c_str(),
                                                      latest_cr.generic_string().c_str(),
                                                      (newer) ? "true" : "false"
                                        );

                                        if (newer) {
                                            MI_CORE_WARN("Latest carnage report is now: {0}",
                                                         file.path().generic_string().c_str()
                                            );

                                            latest_cr = file.path();
                                            latest_cr_write = file.last_write_time();
                                        }
                                    }
                                }
                            }

                            MI_CORE_INFO("Copying carnage report: {0}",
                                         latest_cr.generic_string().c_str());

                            std::filesystem::path to =
                                std::filesystem::path(".\\mccinfo_cache\\autosave") /
                                latest_cr.filename();
                            try {
                                std::filesystem::copy_file(latest_cr, to);
                            }
                            catch (std::exception& e) {
                                MI_CORE_ERROR("Error copying file {0}\nException: {1}",
                                              latest_cr.generic_string().c_str(), e.what()
                                );
                            }


                        
                        }
                    }

                    // here carnage reports are chronologically the last file related to the match written,
                    // therefore on its copy we can also take the autosave cache and make it a match in /matches
                    std::string iso_basename = utility::CurrentTimestampISO();

                    iso_basename.erase(
                        std::remove(iso_basename.begin(), iso_basename.end(), ':'),
                        iso_basename.end());

                    auto match_to = std::filesystem::path(".\\mccinfo_cache\\matches") /
                                    iso_basename;

                    MI_CORE_TRACE(
                        "Attempting to construct match with:\n\tbasename: {0}\n\tpath: {1}",
                        iso_basename.c_str(), match_to.generic_string().c_str());

                    try {

                        std::filesystem::create_directories(match_to);
                        
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));

                        MI_CORE_TRACE("Copying autosave cache to {0}", match_to.generic_string().c_str());

                        std::filesystem::copy(std::filesystem::path(".\\mccinfo_cache\\autosave"),
                                              match_to);
                        for (const auto entry : std::filesystem::directory_iterator(
                            std::filesystem::path(".\\mccinfo_cache\\autosave"))) {

                            if (std::filesystem::is_regular_file(entry.path())) {
                                MI_CORE_WARN("Removing file: {0}",
                                             entry.path().generic_string().c_str());
                                std::filesystem::remove(entry.path());
                            }
                        }

                        MI_CORE_INFO("Success?");

                    } catch (std::exception &e) {
                        MI_CORE_ERROR("Error copying autosave cache\nException: {0}",
                                        e.what());
                    }

                    mi.map = "";

                    emi_.reset();
                    should_copy_cr = false;
                }
            }

            if (should_save_autosave) {
                std::string temp = "C:\\Users\\xbox\\AppData\\LocalLow\\MCC\\Temporary\\";
                std::wstring game = L"";
                mccinfo::game_hint hint = mccinfo::game_hint::HALO3;

                assert(!game_id_sm.is(boost::sml::state<states::none>));

                if (game_id_sm.is(boost::sml::state<states::haloce>)) {
                    temp += "Halo1";
                    game = L"Halo1";
                    hint = mccinfo::game_hint::HALO1;
                    //MessageBox(NULL, L"HALO1", L"", MB_OK);

                } else if (game_id_sm.is(boost::sml::state<states::halo2>)) {
                    temp += "Halo2";
                    game = L"Halo2";

                    hint = mccinfo::game_hint::HALO2;
                    //MessageBox(NULL, L"HALO2", L"", MB_OK);


                } else if (game_id_sm.is(boost::sml::state<states::halo3>)) {
                    temp += "Halo3";
                    game = L"Halo3";

                    hint = mccinfo::game_hint::HALO3;
                    //MessageBox(NULL, L"HALO3", L"", MB_OK);

                } else if (game_id_sm.is(boost::sml::state<states::halo3odst>)) {
                    temp += "Halo3ODST";
                    game = L"Halo3ODST";

                    hint = mccinfo::game_hint::HALO3;
                    //MessageBox(NULL, L"HALO3ODST", L"", MB_OK);

                } else if (game_id_sm.is(boost::sml::state<states::haloreach>)) {
                    temp += "HaloReach";
                    game = L"HaloReach";

                    hint = mccinfo::game_hint::HALOREACH;
                    //MessageBox(NULL, L"HALOREACH", L"", MB_OK);

                } else if (game_id_sm.is(boost::sml::state<states::halo4>)) {
                    temp += "Halo4";
                    game = L"Halo4";
                    hint = mccinfo::game_hint::HALO4;
                    //MessageBox(NULL, L"HALO4", L"", MB_OK);

                } else if (game_id_sm.is(boost::sml::state<states::halo2a>)) {
                    temp += "Halo2A";
                    game = L"Halo2A";
                    hint = mccinfo::game_hint::HALO2A;
                    //MessageBox(NULL, L"HALO2A", L"", MB_OK);
                }

                temp += "\\autosave";
                emi_.game_hint_ = hint;

                autosave_client_.set_copy_src(temp);
                autosave_client_.set_flatten_on_write(true);

                autosave_client_.set_on_complete([this, hint, game](const std::filesystem::path &src, const
                                                                  std::filesystem::path &path) {
                    mccinfo::game_hint hint_ = hint;
                    
                    MI_CORE_TRACE("autosave_client post_callback executed with\n\tsrc_: {0}\n\tdst_: {1}", 
                        src.generic_string().c_str(), 
                        path.generic_string().c_str());
                    
                    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
                        for (const auto &entry : std::filesystem::directory_iterator(path)) {
                            const auto &path2 = entry.path();
                            MI_CORE_TRACE("Looking for theater file data: {0}",
                                          path2.generic_string().c_str()
                            );

                            if (std::filesystem::is_regular_file(path2)) {
                                if ((path2.extension().generic_string() == ".temp") ||
                                    (path2.extension().generic_string() == ".film")) {
                                    MI_CORE_INFO("Setting Extended Match Info Theater File to: {0}", 
                                        path2.generic_string().c_str()
                                    );

                                    try {
                                        this->emi_.theater_file_data_ = file_readers::ReadTheaterFile(std::filesystem::canonical(path2), hint_);
                                    }
                                    catch (std::exception& e) {
                                        MI_CORE_ERROR("fsm controller failed to read theater file data from {0} with exception: {1}", 
                                            std::filesystem::canonical(path2).generic_string().c_str(), 
                                            e.what()
                                        );

                                        this->emi_.theater_file_data_ = std::nullopt;
                                    }
                                }
                            }
                        }
                    }
                });

                //{
                //    while (true) {
                //        // acquire lock
                //        // check op
                //        // if stop, break
                //        // else, request_copy(0)
                //        // this_thread_sleep_for(1000)
                //    }
                //}

                // 5 second delay should be sufficient for h2a/h4
                //if ((hint == mccinfo::file_readers::game_hint::HALO2A) ||
                //    (hint == mccinfo::file_readers::game_hint::HALO4)) {
                //    autosave_client_.request_copy(20000);
                //} else {
                //
                //    autosave_client_.request_copy(1000);
                //}
                autosave_client_.request_copy(1000);

                //autosave_thread_ = std::thread([&] {
                //    while (true) {
                //        std::unique_lock<std::mutex> lock(autosave_mut_);
                //
                //        MI_CORE_TRACE("autosave thread acquired lock");
                //
                //        if (stop_autosave_) {
                //            stop_autosave_ = false;
                //            break;
                //        }
                //
                //        try {
                //            MI_CORE_TRACE("autosave thread requesting copy");
                //            autosave_client_.request_copy(5000);
                //            MI_CORE_WARN("autosave_thread: copy request success");
                //        }
                //        catch (std::exception& e) {
                //            MI_CORE_WARN("autosave_thread: failed to request copy: \n\tException: {0}", e.what());
                //        }
                //    }
                //});

                should_save_autosave = false;
            }

            // catch the carnagereport
            if (user_sm.is(boost::sml::state<states::in_game>) && should_id_cr) {
                if (predicates::events::temp_carnage_report_created(record, trace_context)) {
                    krabs::schema schema(record, trace_context.schema_locator);
                    krabs::parser parser(schema);
                    std::wstring filename = parser.parse<std::wstring>(L"OpenPath");
                    auto bytes = utility::ConvertWStringToBytes(filename);
                    if (bytes.has_value()) {
                        mi.map = bytes.value();
                        emi_.carnage_report_ = bytes.value();
                        emi_.carnage_report_.value().replace_extension();
                        should_id_cr = false;
                    }
                }
            }

        }
    }

    std::string get_map_info() const {
        return mi.map;
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
        utility::PrintTraceEvent(woss, record, trace_context);

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

    void add_match_data_collector_callbacks() {
        cb_table_.add_callback(LOADING_IN | ON_STATE_ENTRY, [&] {
            std::thread cap_t([&] {
                should_id_map = true;

                std::this_thread::sleep_for(std::chrono::milliseconds(4000));

                HWND hwnd = query::LookForMCCWindowHandle().value();

                RECT sr;
                GetWindowRect(hwnd, &sr);

                utility::ScreenCapture(sr, L".\\mccinfo_cache\\autosave\\loading_in.jpeg");
            });

            cap_t.detach();
        });

        cb_table_.add_callback(LOADING_OUT | ON_STATE_ENTRY, [&] {
            std::thread cap_t([&] {
                HWND hwnd = query::LookForMCCWindowHandle().value();

                RECT sr;
                GetWindowRect(hwnd, &sr);

                utility::ScreenCapture(sr, L".\\mccinfo_cache\\autosave\\loading_out.jpeg");
            });

            cap_t.detach();
        });

        cb_table_.add_callback(IN_GAME | ON_STATE_ENTRY, [&] {
            should_save_autosave = true;
            should_id_cr = true;
            should_copy_cr = true;
        });
        
        //cb_table_.add_callback(LOADING_OUT | ON_STATE_ENTRY, [&] {
        //    {
        //        std::unique_lock<std::mutex> lock(autosave_mut_);
        //        stop_autosave_ = true;
        //    }
        //    autosave_thread_.join();
        //});
    }

  private:
    utility::atomic_mutex lock;

    states::state_context sc{};
    details::filtering_context fc{};
    boost::sml::sm<machines::mcc> mcc_sm;
    boost::sml::sm<machines::user> user_sm;
    boost::sml::sm<machines::game_id> game_id_sm;

    callback_table& cb_table_;
    std::thread autosave_thread_;
    std::mutex autosave_mut_;
    bool stop_autosave_ = false;

    mccinfo::file_readers::theater_file_data file_data;
    mccinfo::fsm::autosave_client autosave_client_;

    extended_match_info emi_;

    details::map_info mi;

  private:
    std::optional<query::MCCInstallInfo> steam_install_ = std::nullopt;
    std::optional<query::MCCInstallInfo> msstore_install_ = std::nullopt;


    bool should_id_map = false;
    bool should_save_autosave = false;
    bool should_id_cr = false;
    bool should_copy_cr = false;

  private: // filtering
    bool log_full = false;
    //bool log_full = true;
};
} // namespace fsm
} // namespace mccinfo