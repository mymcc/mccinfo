#pragma once

#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#include <krabs/krabs.hpp> //#include <windows.h>
#undef NOMINMAX
#endif

#include <string>

namespace mccinfo {
namespace fsm {
namespace predicates {
inline auto is_launcher =
    krabs::predicates::property_is(L"ImageFileName", std::string("mcclauncher.exe"));
inline auto is_eac =
    krabs::predicates::property_is(L"ImageFileName", std::string("EasyAntiCheat.exe"));
inline auto is_mcc =
    krabs::predicates::property_is(L"ImageFileName", std::string("MCC-Win64-Shipping.exe"));

inline auto is_launcher_image =
    krabs::predicates::property_contains(L"FileName", std::string("mcclauncher.exe"));

inline auto is_main_menu_bg =
    krabs::predicates::property_icontains(L"OpenPath", std::string("FMS_MainMenu_v2.bk2"));

inline auto init_txt = 
    krabs::predicates::property_icontains(L"OpenPath", std::string("init.txt"));

inline auto hud_scoring = 
    krabs::predicates::property_icontains(L"OpenPath", std::string("hud_scoring"));

inline auto xml_tmp = 
    krabs::predicates::property_icontains(L"OpenPath", std::string(".xml.tmp"));

inline auto sound_file = 
    krabs::predicates::property_icontains(L"OpenPath", std::string(".fsb"));

inline auto sound_file_io_size = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(2048));
inline auto pak_file_io_size = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(65536));
inline auto pak_file_io_size2 = krabs::predicates::property_is(L"IoSize", static_cast<uint32_t>(65536));


// https://learn.microsoft.com/en-us/windows/win32/etw/process-typegroup1
inline auto process_start = krabs::predicates::opcode_is(1); // Start
inline auto process_terminate = krabs::predicates::opcode_is(2); // End
inline auto process_found = krabs::predicates::opcode_is(3); // DCStart

// https://learn.microsoft.com/en-us/windows/win32/etw/image-load
inline auto image_load = krabs::predicates::opcode_is(10);

inline auto file_create = krabs::predicates::opcode_is(64);

inline auto file_read = krabs::predicates::opcode_is(67);

inline krabs::predicates::all_of launcher_start_pred({
    &is_launcher,
    &process_start,
});

inline krabs::predicates::all_of launcher_end_pred({
    &is_launcher,
    &process_terminate,
});

inline krabs::predicates::all_of eac_start_pred({
    &is_eac,
    &process_start,
});

inline krabs::predicates::all_of mcc_process_start({
    &is_mcc,
    &process_start,
});

inline krabs::predicates::all_of mcc_process_found({
    &is_mcc,
    &process_found,
});

inline krabs::predicates::all_of mcc_process_lossed({
    &is_mcc,
    &process_terminate,
});

inline krabs::predicates::all_of mcc_launcher_found({
    &is_launcher,
    &process_found
});

inline krabs::predicates::all_of mcc_launcher_loaded({
    &is_launcher_image,
    &image_load,
});

inline krabs::predicates::all_of main_menu_bg_created({&predicates::is_main_menu_bg,
                                                       &predicates::file_create});

inline krabs::predicates::all_of init_txt_created({&predicates::init_txt,
                                                       &predicates::file_create});

inline krabs::predicates::all_of hud_scoring_created({&predicates::hud_scoring,
                                                       &predicates::file_create});

inline krabs::predicates::all_of xml_tmp_created({&predicates::xml_tmp,
                                                       &predicates::file_create});

inline krabs::predicates::all_of sound_file_read({&predicates::sound_file_io_size,
                                                       &predicates::file_read});

inline krabs::predicates::any_of file_targets({&predicates::is_main_menu_bg,
                                               &predicates::init_txt,
                                               &predicates::hud_scoring,
                                               &predicates::xml_tmp,
                                               &predicates::sound_file});

inline krabs::predicates::all_of accepted_file_creates({
    &predicates::file_create,
    &predicates::file_targets
});

inline krabs::predicates::any_of file_io_sizes({&predicates::sound_file_io_size});

inline krabs::predicates::all_of accepted_file_reads({
    &predicates::file_read,
    &predicates::file_io_sizes
});

inline krabs::predicates::any_of io_ops({&predicates::file_create, &predicates::file_read});
} // namespace predicates
} // namespace fsm2
} // namespace mccinfo