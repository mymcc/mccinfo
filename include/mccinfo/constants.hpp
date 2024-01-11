#pragma once

#include <array>
#include <string>

namespace mccinfo {
namespace constants {
/**
 * @brief The Steam&trade; Application ID for Halo: The Master Chief Collection&trade;.
 *
 * This ID is used by Steam to uniquely identify each game or application in its store.
 */
inline constexpr size_t mcc_steam_app_id = 976730;
/**
 * @brief The executable basename + extension for the Halo: The Master Chief
 * Collection&trade; launcher.
 */
inline constexpr std::wstring_view launcher_exe_w = L"mcclauncher.exe";
inline constexpr std::string_view launcher_exe = "mcclauncher.exe";
/**
 * @brief The executable basename + extension for Easy Anti-Cheat&trade;.
 */
inline constexpr std::wstring_view eac_exe_w = L"EasyAntiCheat.exe";
inline constexpr std::string_view eac_exe = "EasyAntiCheat.exe";
/**
 * @brief The Steam&trade; executable basename + extension for Halo: The Master Chief
 * Collection&trade;.
 */
inline constexpr std::wstring_view mcc_steam_exe_w = L"MCC-Win64-Shipping.exe";
inline constexpr std::string_view mcc_steam_exe = "MCC-Win64-Shipping.exe";
/**
 * @brief The Microsoft Store&trade; executable basename + extension for Halo: The Master Chief
 * Collection&trade;.
 */
inline constexpr std::wstring_view mcc_msstore_exe_w = L"MCCWinStore-Win64-Shipping.exe";
inline constexpr std::string_view mcc_msstore_exe = "MCCWinStore-Win64-Shipping.exe";
/**
 * @brief The relative directory path to the main Halo: The Master Chief Collection&trade;
 * executable within its installation path. That means the absolute path to the executable in the
 * filesystem is:
 * - {Steam MCC Install Path} \ {mcc_relative_path_to_exe_w} \ {mcc_steam_exe_w}
 * - {MS Store MCC Install Path} \ {mcc_relative_path_to_exe_w} \ {mcc_msstore_exe_w}
 */
inline constexpr std::wstring_view mcc_relative_path_to_exe_w = L"mcc\\binaries\\win64";
inline constexpr std::string_view mcc_relative_path_to_exe = "mcc\\binaries\\win64";
/**
 * @brief The temporary directory Halo: The Master Chief Collection&trade; uses.
 */
inline constexpr std::wstring_view mcc_system_temp_path_w = L"%AppData%\\..\\LocalLow\\MCC";
inline constexpr std::string_view mcc_system_temp_path = "%AppData%\\..\\LocalLow\\MCC";
/**
 * @brief The array of all Halo: The Master Chief Collection&trade;'s individual title basenames.
 */
inline constexpr std::array<std::wstring_view, 6> mcc_game_basenames_w = {L"halo1",     L"halo2", L"halo3",
                                                                          L"halo3odst", L"halo4", L"haloreach"};
inline constexpr std::array<std::string_view, 6> mcc_game_basenames    = {"halo1",     "halo2", "halo3",
                                                                          "halo3odst", "halo4", "haloreach"};
/**
 * @brief The number of bytes Halo: The Master Chief Collection&trade; reads from .fsb (sound files) at a time during runtime.
 */
inline constexpr size_t fsb_fio_read_size = 2048;
/**
 * @brief The number of bytes Halo: The Master Chief Collection&trade; reads from its .pak (asset package) at a time during runtime.
 */
inline constexpr size_t pak_fio_read_size = 65536;
/**
 * @brief The number of bytes Halo: The Master Chief Collection&trade; reads from its .pak (asset package) at a time during runtime.
 */
inline constexpr size_t bk2_fio_read_size = 131072;

} // namespace mccinfo
}