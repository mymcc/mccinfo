#pragma once

#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#include <krabs/krabs.hpp> //#include <windows.h>
#undef NOMINMAX
#endif

#include <string>
#include "mccinfo/constants.hpp"

namespace mccinfo {
namespace fsm {
namespace predicates {
namespace opcodes {

// https://learn.microsoft.com/en-us/windows/win32/etw/process-typegroup1
enum class process : uint8_t {
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

// https://learn.microsoft.com/en-us/windows/win32/etw/image-load
enum class image : uint8_t {
    load = 10,
    unload = 2,
    dc_start = 3,
    dc_end = 4,
};

} // opcodes

namespace process {

inline auto start               = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::process::start));
inline auto end                 = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::process::end));
inline auto alive               = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::process::dc_start));
inline auto alive_at_trace_end  = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::process::dc_end));

} // process

namespace fio {

inline auto file_name           = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_name));
inline auto file_name_create    = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_create));
inline auto file_name_delete    = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_name_delete));
inline auto file_name_rundown   = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_name_rundown));
inline auto file_create         = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_create));
inline auto file_read           = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_read));
inline auto file_write          = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::fio::file_write));

} // fio

namespace image {

inline auto load                = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::image::load));
inline auto unload              = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::image::unload));
inline auto loaded              = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::image::dc_start));
inline auto loaded_at_trace_end = krabs::predicates::opcode_is(static_cast<uint8_t>(opcodes::image::dc_end));

} // image

namespace likely {

inline auto is_launcher         = krabs::predicates::property_is(L"ImageFileName", std::string(constants::launcher_exe));
inline auto is_eac              = krabs::predicates::property_is(L"ImageFileName", std::string(constants::eac_exe));
inline auto is_steam_mcc        = krabs::predicates::property_is(L"ImageFileName", std::string(constants::mcc_steam_exe));
inline auto is_msstore_mcc      = krabs::predicates::property_is(L"ImageFileName", std::string(constants::mcc_msstore_exe));
inline auto is_mcc              = krabs::predicates::any_of({ &is_steam_mcc, &is_msstore_mcc});

inline auto is_temp_carnage_report = krabs::predicates::property_icontains(L"OpenPath", std::string(".xml.tmp"));
inline auto match_init_file     = krabs::predicates::property_icontains(L"OpenPath", std::string("init.txt"));
inline auto sound_file          = krabs::predicates::property_icontains(L"OpenPath", std::string(".fsb"));
inline auto hud_scoring_gfx_file = krabs::predicates::property_icontains(L"OpenPath", std::string("hud_scoring"));
inline auto is_main_menu_background_video_file = krabs::predicates::property_icontains(L"OpenPath", std::string("FMS_MainMenu_v2.bk2"));
inline auto is_sound_file_read = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(constants::fsb_fio_read_size));
inline auto is_pak_file_read = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(constants::pak_fio_read_size));

inline auto is_launcher_image = krabs::predicates::property_contains(L"FileName", std::string(constants::launcher_exe));

} // likely

namespace events {

inline krabs::predicates::all_of launcher_started({
    &likely::is_launcher,
    &process::start,
});

inline krabs::predicates::all_of launcher_terminated({
    &likely::is_launcher,
    &process::end,
});

inline krabs::predicates::all_of launcher_found({
    &likely::is_launcher,
    &process::alive
});

inline krabs::predicates::all_of eac_started({
    &likely::is_eac,
    &process::start,
});

inline krabs::predicates::all_of mcc_started({
    &likely::is_mcc,
    &process::start,
});

inline krabs::predicates::all_of mcc_terminated({
    &likely::is_mcc,
    &process::end,
});

inline krabs::predicates::all_of mcc_found({
    &likely::is_mcc,
    &process::alive,
});

inline krabs::predicates::all_of sound_file_read({
    &likely::is_sound_file_read,
    &fio::file_read
});

inline krabs::predicates::all_of main_menu_background_video_file_created({
    &likely::is_main_menu_background_video_file,
    &fio::file_create
});

inline krabs::predicates::all_of match_init_file_created({
    &likely::match_init_file,
    &fio::file_create
});

inline krabs::predicates::all_of hud_scoring_gfx_file_created({
    &likely::hud_scoring_gfx_file,
    &fio::file_create
});

inline krabs::predicates::all_of temp_carnage_report_created({
    &likely::is_temp_carnage_report,
    &fio::file_create
});

inline krabs::predicates::all_of mcc_launcher_loaded({
    &likely::is_launcher_image,
    &image::load,
});

} // events

namespace filters {

inline krabs::predicates::any_of file_create_targets({
    &likely::is_main_menu_background_video_file,
    &likely::match_init_file,
    &likely::hud_scoring_gfx_file,
    &likely::is_temp_carnage_report,
    &likely::sound_file
});

inline krabs::predicates::all_of accepted_file_creates({
    &fio::file_create,
    &file_create_targets
});

inline krabs::predicates::any_of file_io_sizes({
    &likely::is_sound_file_read,
    &likely::is_pak_file_read
});

inline krabs::predicates::all_of accepted_file_reads({
    &fio::file_read,
    &file_io_sizes
});

inline constexpr auto make_process_filter = []() {
    return krabs::event_filter{
        krabs::predicates::any_of({
            &likely::is_launcher,
            &likely::is_eac,
            &likely::is_mcc
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

inline constexpr auto make_image_filter = []() {
    return krabs::event_filter{
        krabs::predicates::all_of({
            &likely::is_launcher_image, 
            &image::load
        })
    };
};
} // namespace filters
} // namespace predicates
} // namespace fsm
} // namespace mccinfo