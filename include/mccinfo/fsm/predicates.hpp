#pragma once

#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#include <krabs/krabs.hpp> //#include <windows.h>
#undef NOMINMAX
#endif

#include <string>
#include "mccinfo/constants.hpp"

#define CREATE_PREDICATE(condition, target) \
inline krabs::predicates::all_of target ({\
    &likely_is::target,\
    &condition});

namespace mccinfo {
namespace fsm {
namespace predicates {
namespace opcodes {

enum class process : uint8_t {
// https://learn.microsoft.com/en-us/windows/win32/etw/process-typegroup1
    start = 1,
    end = 2,
    dc_start = 3,
    dc_end = 4,
};

enum class fio : uint8_t {
// https://learn.microsoft.com/en-us/windows/win32/etw/fileio-name
    file_name = 0,
    file_name_create = 32,
    file_name_delete = 35,
    file_name_rundown = 36,

// https://learn.microsoft.com/en-us/windows/win32/etw/fileio-create
    file_create = 64,

// https://learn.microsoft.com/en-us/windows/win32/etw/fileio-readwrite
    file_read = 67,
    file_write = 68,
};

enum class image : uint8_t {
// https://learn.microsoft.com/en-us/windows/win32/etw/image-load
    load = 10,
    unload = 2,
    dc_start = 3,
    dc_end = 4,
};

enum class handle : uint8_t {
    create = 32,
    close = 33,
    type_dc_start = 36,
    type_dc_end = 37,
    dc_start = 38,
    dc_end = 39,

};

} // opcodes

namespace process {

inline auto start                = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::process::start));
inline auto end                  = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::process::end));
inline auto alive_at_trace_start = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::process::dc_start));
inline auto alive_at_trace_end   = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::process::dc_end));

} // process

namespace fio {

inline auto file_name           = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_name));
inline auto file_name_create    = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_name_create));
inline auto file_name_delete    = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_name_delete));
inline auto file_name_rundown   = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_name_rundown));
inline auto file_create         = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_create));
inline auto file_read           = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_read));
inline auto file_write          = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_write));

} // fio

namespace image {

inline auto load                  = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::image::load));
inline auto unload                = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::image::unload));
inline auto loaded_at_trace_start = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::image::dc_start));
inline auto loaded_at_trace_end   = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::image::dc_end));

} // image

namespace handle {

inline auto create                   = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::handle::create));
inline auto close                    = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::handle::close));
inline auto type_open_at_trace_start = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::handle::type_dc_start));
inline auto type_open_at_trace_end   = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::handle::type_dc_end));
inline auto open_at_trace_start      = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::handle::dc_start));
inline auto open_at_trace_end        = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::handle::dc_end));

} // handle
namespace likely_is {

inline auto launcher         = krabs::predicates::property_is(L"ImageFileName", std::string(constants::launcher_exe));
inline auto eac              = krabs::predicates::property_is(L"ImageFileName", std::string(constants::eac_exe));
inline auto steam_mcc        = krabs::predicates::property_is(L"ImageFileName", std::string(constants::mcc_steam_exe));
inline auto msstore_mcc      = krabs::predicates::property_is(L"ImageFileName", std::string(constants::mcc_msstore_exe));
inline auto mcc              = krabs::predicates::any_of({ &steam_mcc, &msstore_mcc});


inline auto map_file = krabs::predicates::property_icontains(L"OpenPath", std::string(".map"));
inline auto map_info_file = krabs::predicates::property_icontains(L"OpenPath", std::string(".mapinfo"));
inline auto hdmu_map_file = krabs::predicates::property_icontains(L"OpenPath", std::string("hdmu.map"));
inline auto shared_map_file = krabs::predicates::property_icontains(L"OpenPath", std::string("shared.map"));
inline auto campaign_map_file = krabs::predicates::property_icontains(L"OpenPath", std::string("campaign.map"));
inline auto mainmenu_map_file = krabs::predicates::property_icontains(L"OpenPath", std::string("mainmenu.map"));
inline auto theater_file = krabs::predicates::property_icontains(L"OpenPath", std::string(".mov"));
inline auto temp_carnage_report = krabs::predicates::property_icontains(L"OpenPath", std::string(".xml.tmp"));
inline auto backup_carnage_report = krabs::predicates::property_icontains(L"OpenPath", std::string(".xml.bak"));
inline auto match_init_file     = krabs::predicates::property_icontains(L"OpenPath", std::string("init.txt"));
inline auto match_launch_file   = krabs::predicates::property_icontains(L"OpenPath", std::string("launch.txt"));
inline auto sound_file          = krabs::predicates::property_icontains(L"OpenPath", std::string(".fsb"));
inline auto hud_scoring_gfx_file = krabs::predicates::property_icontains(L"OpenPath", std::string("hud_scoring.gfx"));
inline auto loadingscreen_gfx_file = krabs::predicates::property_icontains(L"OpenPath", std::string("loadingscreen.gfx"));
inline auto restartscreen_gfx_file = krabs::predicates::property_icontains(L"OpenPath", std::string("restartscreen.gfx"));
inline auto paused_game_gfx_file = krabs::predicates::property_icontains(L"OpenPath", std::string("multiplayerpausedgame.gfx"));
inline auto soundstream_pck_file = krabs::predicates::property_icontains(L"OpenPath", std::string("soundstream.pck"));
inline auto match_temp_file = krabs::predicates::property_icontains(L"OpenPath", std::string(".temp"));

inline auto halo2a_autosave_temp_file = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Temporary\\Halo2A\\autosave"));
inline auto halo3_autosave_temp_file = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Temporary\\Halo3\\autosave"));
inline auto halo3odst_autosave_temp_file = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Temporary\\Halo3ODST\\autosave"));
inline auto halo4_autosave_temp_file = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Temporary\\Halo4\\autosave"));
inline auto haloreach_autosave_temp_file = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Temporary\\HaloReach\\autosave"));

inline auto halo2_autosave_bin_file = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Config\\autosave_Halo2.bin"));
inline auto halo3_autosave_bin_file = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Config\\autosave_Halo3.bin"));

inline auto haloce_lang_bin = krabs::predicates::property_icontains(L"OpenPath", std::string("_Halo1.bin"));
inline auto halo2_lang_bin = krabs::predicates::property_icontains(L"OpenPath", std::string("_Halo2.bin"));
inline auto halo2a_lang_bin = krabs::predicates::property_icontains(L"OpenPath", std::string("_Halo2A.bin"));
inline auto halo3_lang_bin = krabs::predicates::property_icontains(L"OpenPath", std::string("_Halo3.bin"));
inline auto halo3odst_lang_bin = krabs::predicates::property_icontains(L"OpenPath", std::string("_Halo3ODST.bin"));
inline auto halo4_lang_bin = krabs::predicates::property_icontains(L"OpenPath", std::string("_Halo4.bin"));
inline auto haloreach_lang_bin = krabs::predicates::property_icontains(L"OpenPath", std::string("_HaloReach.bin"));


inline auto main_menu_background_video_file = krabs::predicates::property_icontains(L"OpenPath", std::string("FMS_MainMenu_v2.bk2"));
inline auto ms_logo_background_video_file = krabs::predicates::property_icontains(L"OpenPath", std::string("FMS_logo_microsoft_7_1_.bk2"));


inline auto background_video_file = krabs::predicates::property_icontains(L"OpenPath", std::string(".bk2"));
inline auto sound_file_read = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(constants::fsb_fio_read_size));
inline auto halo1_initial_sound_file_read = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(constants::halo1_initial_fsb_read_size));

inline auto pak_file_read = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(constants::pak_fio_read_size));
inline auto bk2_file_read = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(constants::bk2_fio_read_size));
inline auto font_package_file_read = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(constants::font_package_read_size));

inline auto launcher_image = krabs::predicates::property_icontains(L"FileName", std::string(constants::launcher_exe));
inline auto cryptui_image = krabs::predicates::property_icontains(L"FileName", std::string("cryptui.dll"));
inline auto wininet_image = krabs::predicates::property_icontains(L"FileName", std::string("wininet.dll"));
inline auto winrnr_image = krabs::predicates::property_icontains(L"FileName", std::string("winrnr.dll"));
inline auto wsock32_image = krabs::predicates::property_icontains(L"FileName", std::string("wsock32.dll"));
inline auto partywin_image = krabs::predicates::property_icontains(L"FileName", std::string("PartyWin.dll"));

inline auto halo1_image = krabs::predicates::property_icontains(L"FileName", std::string("halo1.dll"));
inline auto halo2_image = krabs::predicates::property_icontains(L"FileName", std::string("halo2.dll"));
inline auto halo3_image = krabs::predicates::property_icontains(L"FileName", std::string("halo3.dll"));
inline auto halo3_odst_image = krabs::predicates::property_icontains(L"FileName", std::string("halo3odst.dll"));
inline auto halo4_image = krabs::predicates::property_icontains(L"FileName", std::string("halo4.dll"));
inline auto halo_reach_image = krabs::predicates::property_icontains(L"FileName", std::string("haloreach.dll"));
inline auto groundhog_image = krabs::predicates::property_icontains(L"FileName", std::string("groundhog.dll"));

} // likely_is

namespace contains {
    inline auto halo1 = krabs::predicates::property_icontains(L"OpenPath", std::string("halo1"));
    inline auto halo2 = krabs::predicates::property_icontains(L"OpenPath", std::string("halo2"));
    inline auto mpcarnagereport = krabs::predicates::property_icontains(L"OpenPath", std::string("mpcarnagereport"));
    inline auto survivalcarnagereport = krabs::predicates::property_icontains(L"OpenPath", std::string("survivalcarnagereport"));
    inline auto h2a_movie_path = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Temporary\\UserContent\\Halo2A\\Movie"));
    inline auto h3_movie_path = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Temporary\\UserContent\\Halo3\\Movie"));
    inline auto h3odst_movie_path = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Temporary\\UserContent\\Halo3ODST\\Movie"));
    inline auto h4_movie_path = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Temporary\\UserContent\\Halo4\\Movie"));
    inline auto reach_movie_path = krabs::predicates::property_icontains(L"OpenPath", std::string("MCC\\Temporary\\UserContent\\HaloReach\\Movie"));
    inline auto ui_localization_path = krabs::predicates::property_icontains(L"OpenPath", std::string("data\\ui\\Localization"));
    inline auto shared = krabs::predicates::property_icontains(L"OpenPath", std::string("shared"));
    inline auto cache = krabs::predicates::property_icontains(L"OpenPath", std::string("cache"));
    inline auto campaign = krabs::predicates::property_icontains(L"OpenPath", std::string("campaign"));
}

namespace certainly_not {

inline krabs::predicates::none_of wininet_image({
    &likely_is::wininet_image,
});

inline krabs::predicates::none_of map_info_file({
    &likely_is::map_info_file,
});

inline krabs::predicates::none_of shared_map_file({
    &likely_is::shared_map_file,
});

inline krabs::predicates::none_of cache_map_file({
    &contains::cache,
});

inline krabs::predicates::none_of campaign_map_file({
    &likely_is::campaign_map_file,
});

inline krabs::predicates::none_of hdmu_map_file({
    &likely_is::hdmu_map_file,
});

inline krabs::predicates::none_of mainmenu_map_file({
    &likely_is::mainmenu_map_file,
});

inline krabs::predicates::none_of backup_carnage_report({
    &likely_is::backup_carnage_report,
});

inline krabs::predicates::none_of temp_carnage_report({
    &likely_is::temp_carnage_report,
});

} // certainly_not
namespace events {

inline krabs::predicates::all_of launcher_started({
    &likely_is::launcher,
    &process::start,
});

inline krabs::predicates::all_of launcher_terminated({
    &likely_is::launcher,
    &process::end,
});

inline krabs::predicates::all_of launcher_found({
    &likely_is::launcher,
    &process::alive_at_trace_start
});

inline krabs::predicates::all_of eac_started({
    &likely_is::eac,
    &process::start,
});

inline krabs::predicates::all_of mcc_started({
    &likely_is::mcc,
    &process::start,
});

inline krabs::predicates::all_of mcc_terminated({
    &likely_is::mcc,
    &process::end,
});

inline krabs::predicates::all_of mcc_found({
    &likely_is::mcc,
    &process::alive_at_trace_start,
});

inline krabs::predicates::all_of sound_file_read({
    &likely_is::sound_file_read,
    &fio::file_read
});

inline krabs::predicates::all_of halo1_initial_sound_file_read({
    &likely_is::halo1_initial_sound_file_read,
    &fio::file_read
});

inline krabs::predicates::all_of bg_video_file_read({
    &likely_is::bk2_file_read,
    &fio::file_read
});

inline krabs::predicates::all_of font_package_file_read({
    &likely_is::font_package_file_read,
    &fio::file_read
});

inline krabs::predicates::none_of not_ms_logo_video_file({
    &likely_is::ms_logo_background_video_file,
});

inline krabs::predicates::all_of in_menus_video_file_read({
    &not_ms_logo_video_file,
    &bg_video_file_read
});

inline krabs::predicates::all_of main_menu_background_video_file_created({
    &likely_is::main_menu_background_video_file,
    &fio::file_create
});

inline krabs::predicates::all_of map_file_created({
    &likely_is::map_file,
    &certainly_not::map_info_file,
    &certainly_not::cache_map_file,
    &certainly_not::shared_map_file,
    &certainly_not::campaign_map_file,
    &certainly_not::hdmu_map_file,
    &certainly_not::mainmenu_map_file,
    &fio::file_create
});

inline krabs::predicates::all_of match_init_file_created({
    &likely_is::match_init_file,
    &fio::file_create
});

inline krabs::predicates::all_of hud_scoring_gfx_file_created({
    &likely_is::hud_scoring_gfx_file,
    &fio::file_create
});

inline krabs::predicates::all_of restartscreen_gfx_file_created({
    &likely_is::restartscreen_gfx_file,
    &fio::file_create
});

inline krabs::predicates::all_of loadingscreen_gfx_file_created({
    &likely_is::loadingscreen_gfx_file,
    &fio::file_create
});

inline krabs::predicates::all_of paused_game_gfx_file_created({
    &likely_is::paused_game_gfx_file,
    &fio::file_create
});

inline krabs::predicates::all_of temp_carnage_report_created({
    &likely_is::temp_carnage_report,
    &fio::file_create
});

inline krabs::predicates::all_of mp_temp_carnage_report_created({
    &likely_is::temp_carnage_report,
    &contains::mpcarnagereport,
    &fio::file_create
});

inline krabs::predicates::all_of survival_temp_carnage_report_created({
    &likely_is::temp_carnage_report,
    &contains::survivalcarnagereport,
    &fio::file_create
});

inline krabs::predicates::all_of backup_temp_carnage_report_created({
    &likely_is::backup_carnage_report,
    &fio::file_create
});

inline krabs::predicates::all_of soundstream_pck_file_created({
    &likely_is::soundstream_pck_file,
    &fio::file_create
});

inline krabs::predicates::all_of halo1_match_init_file_created({
    &likely_is::match_init_file,
    &contains::halo1,
    &fio::file_create
});

inline krabs::predicates::all_of halo2_match_launch_file_created({
    &likely_is::match_launch_file,
    &contains::halo2,
    &fio::file_create
});

inline krabs::predicates::all_of match_temp_file_created({
    &likely_is::match_temp_file,
    &fio::file_create
});

inline krabs::predicates::all_of halo2a_autosave_temp_file_created({
    &likely_is::halo2a_autosave_temp_file,
    &fio::file_create
});

inline krabs::predicates::all_of halo3_autosave_temp_file_created({
    &likely_is::halo3_autosave_temp_file,
    &fio::file_create
});

inline krabs::predicates::all_of halo3odst_autosave_temp_file_created({
    &likely_is::halo3odst_autosave_temp_file,
    &fio::file_create
});

inline krabs::predicates::all_of halo4_autosave_temp_file_created({
    &likely_is::halo4_autosave_temp_file,
    &fio::file_create
});

inline krabs::predicates::all_of haloreach_autosave_temp_file_created({
    &likely_is::haloreach_autosave_temp_file,
    &fio::file_create
});

inline krabs::predicates::all_of halo2a_theater_file_created({
    &likely_is::theater_file,
    &contains::h2a_movie_path,
    &fio::file_create
});

inline krabs::predicates::all_of halo3_theater_file_created({
    &likely_is::theater_file,
    &contains::h3_movie_path,
    &fio::file_create
});

inline krabs::predicates::all_of halo3odst_theater_file_created({
    &likely_is::theater_file,
    &contains::h3odst_movie_path,
    &fio::file_create
});

inline krabs::predicates::all_of halo4_theater_file_created({
    &likely_is::theater_file,
    &contains::h4_movie_path,
    &fio::file_create
});

inline krabs::predicates::all_of haloreach_theater_file_created({
    &likely_is::theater_file,
    &contains::reach_movie_path,
    &fio::file_create
});

inline krabs::predicates::all_of halo2_autosave_bin_file_created({
    &likely_is::halo2_autosave_bin_file,
    &fio::file_create
});

inline krabs::predicates::all_of halo3_autosave_bin_file_created({
    &likely_is::halo3_autosave_bin_file,
    &fio::file_create
});

inline krabs::predicates::all_of haloce_lang_bin_file_created({
    &likely_is::haloce_lang_bin,
    &contains::ui_localization_path,
    &fio::file_create
});

inline krabs::predicates::all_of halo2_lang_bin_file_created({
    &likely_is::halo2_lang_bin,
    &contains::ui_localization_path,
    &fio::file_create
});

inline krabs::predicates::all_of halo2a_lang_bin_file_created({
    &likely_is::halo2a_lang_bin,
    &contains::ui_localization_path,
    &fio::file_create
});

inline krabs::predicates::all_of halo3_lang_bin_file_created({
    &likely_is::halo3_lang_bin,
    &contains::ui_localization_path,
    &fio::file_create
});

inline krabs::predicates::all_of halo3odst_lang_bin_file_created({
    &likely_is::halo3odst_lang_bin,
    &contains::ui_localization_path,
    &fio::file_create
});

inline krabs::predicates::all_of halo4_lang_bin_file_created({
    &likely_is::halo4_lang_bin,
    &contains::ui_localization_path,
    &fio::file_create
});

inline krabs::predicates::all_of haloreach_lang_bin_file_created({
    &likely_is::haloreach_lang_bin,
    &contains::ui_localization_path,
    &fio::file_create
});

inline krabs::predicates::all_of mcc_launcher_loaded({
    &likely_is::launcher_image,
    &image::load,
});

namespace loaded_at_trace_start {

CREATE_PREDICATE(image::loaded_at_trace_start, cryptui_image)
CREATE_PREDICATE(image::loaded_at_trace_start, wininet_image)
CREATE_PREDICATE(image::loaded_at_trace_start, winrnr_image)
CREATE_PREDICATE(image::loaded_at_trace_start, wsock32_image)
CREATE_PREDICATE(image::loaded_at_trace_start, partywin_image)

} // loaded_at_trace_start

} // events

namespace filters {

inline krabs::predicates::any_of file_create_targets({
    &likely_is::main_menu_background_video_file,
    &likely_is::background_video_file,
    &likely_is::match_init_file,
    &likely_is::match_launch_file,
    &likely_is::hud_scoring_gfx_file,
    &likely_is::restartscreen_gfx_file,
    &likely_is::loadingscreen_gfx_file,
    &likely_is::temp_carnage_report,
    &likely_is::backup_carnage_report,
    &likely_is::soundstream_pck_file,
    &likely_is::sound_file,
    &likely_is::map_file,
    &likely_is::map_info_file,
    &likely_is::match_temp_file,
    &likely_is::halo3_autosave_bin_file,
    &likely_is::halo2a_autosave_temp_file,
    &likely_is::halo3_autosave_temp_file,
    &likely_is::halo3odst_autosave_temp_file,
    &likely_is::halo4_autosave_temp_file,
    &likely_is::haloreach_autosave_temp_file,
    &likely_is::theater_file,
    &likely_is::haloce_lang_bin,
    &likely_is::halo2_lang_bin,
    &likely_is::halo2a_lang_bin,
    &likely_is::halo3_lang_bin,
    &likely_is::halo3odst_lang_bin,
    &likely_is::halo4_lang_bin,
    &likely_is::haloreach_lang_bin
});

inline krabs::predicates::all_of accepted_file_creates({
    &fio::file_create,
    &file_create_targets
});

inline krabs::predicates::all_of accepted_file_deletes({
    &fio::file_name_delete,
    &likely_is::backup_carnage_report
});

inline krabs::predicates::any_of file_io_sizes({
    &likely_is::sound_file_read,
    //&likely_is::pak_file_read,
    //&likely_is::bk2_file_read,
    &likely_is::font_package_file_read,
    &likely_is::halo1_initial_sound_file_read
});

inline krabs::predicates::all_of accepted_file_reads({
    &fio::file_read,
    &file_io_sizes
});

inline krabs::predicates::any_of image_load_targets({
    &likely_is::wininet_image,
    &likely_is::wsock32_image,
    &likely_is::cryptui_image,
    &likely_is::winrnr_image,
    &likely_is::halo1_image,
    &likely_is::halo2_image,
    &likely_is::halo3_image,
    &likely_is::halo3_odst_image,
    &likely_is::halo4_image,
    &likely_is::halo_reach_image,
    &likely_is::groundhog_image,
    &likely_is::partywin_image,
});

inline krabs::predicates::all_of accepted_image_loads({
    &image_load_targets,
    &image::loaded_at_trace_start
});

inline constexpr auto make_process_filter = []() {
    return krabs::event_filter{
        krabs::predicates::any_of({
            &likely_is::launcher,
            &likely_is::eac,
            &likely_is::mcc
        })
    };
};

inline constexpr auto make_fiio_filter = []() {
    return krabs::event_filter{
        krabs::predicates::any_of({
            &accepted_file_creates,
            &accepted_file_reads
        })
    };
};

inline constexpr auto make_dummy_image_filter = []() {
    return krabs::event_filter{
        krabs::predicates::no_event
    };
};

inline constexpr auto make_handle_filter = []() {
    return krabs::event_filter{
        krabs::predicates::any_of({
            //&handle::create,
            //&handle::close,
            //&handle::type_open_at_trace_start,
            //&handle::type_open_at_trace_end,
            &handle::open_at_trace_start,
            &handle::open_at_trace_end
        })
    };
};
} // namespace filters
} // namespace predicates
} // namespace fsm
} // namespace mccinfo