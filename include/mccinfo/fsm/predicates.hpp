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

inline auto temp_carnage_report = krabs::predicates::property_icontains(L"OpenPath", std::string(".xml.tmp"));
inline auto match_init_file     = krabs::predicates::property_icontains(L"OpenPath", std::string("init.txt"));
inline auto sound_file          = krabs::predicates::property_icontains(L"OpenPath", std::string(".fsb"));
inline auto hud_scoring_gfx_file = krabs::predicates::property_icontains(L"OpenPath", std::string("hud_scoring.gfx"));
inline auto restartscreen_gfx_file = krabs::predicates::property_icontains(L"OpenPath", std::string("restartscreen.gfx"));
inline auto paused_game_gfx_file = krabs::predicates::property_icontains(L"OpenPath", std::string("multiplayerpausedgame.gfx"));

inline auto main_menu_background_video_file = krabs::predicates::property_icontains(L"OpenPath", std::string("FMS_MainMenu_v2.bk2"));
// we want the bg video file that is not the
inline auto ms_logo_background_video_file = krabs::predicates::property_icontains(L"OpenPath", std::string("FMS_logo_microsoft_7_1_.bk2"));
inline auto background_video_file = krabs::predicates::property_icontains(L"OpenPath", std::string(".bk2"));
inline auto sound_file_read = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(constants::fsb_fio_read_size));
inline auto pak_file_read = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(constants::pak_fio_read_size));
inline auto bk2_file_read = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(constants::bk2_fio_read_size));

inline auto launcher_image = krabs::predicates::property_icontains(L"FileName", std::string(constants::launcher_exe));
inline auto cryptui_image = krabs::predicates::property_icontains(L"FileName", std::string("cryptui.dll"));
inline auto wininet_image = krabs::predicates::property_icontains(L"FileName", std::string("wininet.dll"));
inline auto winrnr_image = krabs::predicates::property_icontains(L"FileName", std::string("winrnr.dll"));
inline auto wsock32_image = krabs::predicates::property_icontains(L"FileName", std::string("wsock32.dll"));

} // likely_is

namespace certainly_not {
inline krabs::predicates::none_of wininet_image({
    &likely_is::wininet_image,
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

inline krabs::predicates::all_of bg_video_file_read({
    &likely_is::bk2_file_read,
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

inline krabs::predicates::all_of paused_game_gfx_file_created({
    &likely_is::paused_game_gfx_file,
    &fio::file_create
});

inline krabs::predicates::all_of temp_carnage_report_created({
    &likely_is::temp_carnage_report,
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

} // loaded_at_trace_start

} // events

namespace filters {

inline krabs::predicates::any_of file_create_targets({
    &likely_is::main_menu_background_video_file,
    &likely_is::background_video_file,
    &likely_is::match_init_file,
    &likely_is::hud_scoring_gfx_file,
    &likely_is::restartscreen_gfx_file,
    &likely_is::temp_carnage_report,
    &likely_is::sound_file
});


inline krabs::predicates::all_of accepted_file_creates({
    &fio::file_create,
    &file_create_targets
});

inline krabs::predicates::any_of file_io_sizes({
    &likely_is::sound_file_read,
    &likely_is::pak_file_read,
    &likely_is::bk2_file_read
});

inline krabs::predicates::all_of accepted_file_reads({
    &fio::file_read,
    &file_io_sizes
});

inline krabs::predicates::any_of image_load_targets({
    &likely_is::wininet_image,
    &likely_is::wsock32_image,
    &likely_is::cryptui_image,
    &likely_is::winrnr_image
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
            &accepted_file_creates//,
            //&accepted_file_reads
        })
    };
};

inline constexpr auto make_image_filter = []() {
    return krabs::event_filter{
        krabs::predicates::all_of({
            &accepted_image_loads
            //&image_load_targets
        })
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