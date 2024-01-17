#pragma once

#include <array>
#include <string>
#include <vector>

#include "frozen/unordered_map.h"
namespace mccinfo {
namespace constants {

#define BITFLAG(x) \
    (1ULL << x)

#define MASK_BITS_UP_TO(x) \
    ((1ULL << (x + 1)) - 1)

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
inline constexpr std::array<std::wstring_view, 6> mcc_game_basenames_w = {
    L"halo1", L"halo2", L"halo3", L"halo3odst", L"halo4", L"haloreach"};
inline constexpr std::array<std::string_view, 6> mcc_game_basenames = {
    "halo1", "halo2", "halo3", "halo3odst", "halo4", "haloreach"};

namespace background_videos {
namespace menu {

enum video_keys : uint64_t {
    ANVIL                    = BITFLAG(0),
    COVENANT_WAR             = BITFLAG(1),
    ELITE                    = BITFLAG(2),
    FLOOD_VIGNETTE           = BITFLAG(3),
    FMS_CR_BG                = BITFLAG(4),
    FMS_H1_BG                = BITFLAG(5),
    FMS_H2_BG                = BITFLAG(6),
    FMS_H3_BG                = BITFLAG(7),
    FMS_H3_ODST_BG           = BITFLAG(8),
    FMS_H4_BG                = BITFLAG(9),
    FMS_HR_BG                = BITFLAG(10),
    FMS_LOADING              = BITFLAG(11),
    FMS_LOGO_343_7_1         = BITFLAG(12),
    FMS_LOGO_MICROSOFT_7_1   = BITFLAG(13),
    FMS_MAINMENU             = BITFLAG(14),
    FMS_MAINMENU_V2          = BITFLAG(15),
    FMS_MP_BG                = BITFLAG(16),
    FMS_PLAYER_CUSTOMIZATION = BITFLAG(17),
    FMS_POSTMATCH            = BITFLAG(18),
    H2_VIGNETTE              = BITFLAG(19),
    H3_ODST_VIGNETTE         = BITFLAG(20),
    H3_VIGNETTE              = BITFLAG(21),
    H4_VIGNETTE              = BITFLAG(22),
    HALO_2_ANNIVERSARY       = BITFLAG(23),
    HALO_CE_ANNIVERSARY      = BITFLAG(24),
    HALO_REACH_VIGNETTE      = BITFLAG(25),
    HALO_RING_VIGNETTE       = BITFLAG(26),
    HCE_VIGNETTE             = BITFLAG(27),
    MAINMENU_ACE             = BITFLAG(28),
    MAINMENU_OFF             = BITFLAG(29),
    MYTHIC                   = BITFLAG(30),
    NOBLE                    = BITFLAG(31),
    RAVEN                    = BITFLAG(32),
    RECLAIMER                = BITFLAG(33),
    RECON                    = BITFLAG(34),
    SPARK                    = BITFLAG(35),
    WARTHOG_VIGNETTE         = BITFLAG(36),
    YAPYAP_VIGNETTE          = BITFLAG(37),
    ALL                      = MASK_BITS_UP_TO(37)
};
} // menu
} // background_videos
} // constants
} // mccfsm

namespace frozen {
    template <> struct elsa<mccinfo::constants::background_videos::menu::video_keys> : elsa<uint64_t> {
    };
}
namespace mccinfo {
namespace constants {
namespace background_videos {
namespace menu {

frozen::unordered_map<video_keys, std::wstring_view, 38> basenames_w = {
    {video_keys::ANVIL,                    L"anvil.bk2"},
    {video_keys::COVENANT_WAR,             L"CovenantWar.bk2"}, 
    {video_keys::ELITE,                    L"ELITE.bk2"},
    {video_keys::FLOOD_VIGNETTE,           L"FloodVignette.bk2"},
    {video_keys::FMS_CR_BG,                L"FMS_cr_bg.bk2"}, 
    {video_keys::FMS_H1_BG,                L"FMS_h1_bg.bk2"},
    {video_keys::FMS_H2_BG,                L"FMS_h2_bg.bk2"}, 
    {video_keys::FMS_H3_BG,                L"FMS_h3_bg.bk2"},
    {video_keys::FMS_H3_ODST_BG,           L"FMS_h3odst_bg.bk2"},
    {video_keys::FMS_H4_BG,                L"FMS_h4_bg.bk2"}, 
    {video_keys::FMS_HR_BG,                L"FMS_hr_bg.bk2"},
    {video_keys::FMS_LOADING,              L"FMS_loading.bk2"},
    {video_keys::FMS_LOGO_343_7_1,         L"FMS_logo_343_7_1_.bk2"},
    {video_keys::FMS_LOGO_MICROSOFT_7_1,   L"FMS_logo_microsoft_7_1_.bk2"},
    {video_keys::FMS_MAINMENU,             L"FMS_MainMenu.bk2"},
    {video_keys::FMS_MAINMENU_V2,          L"FMS_MainMenu_v2.bk2"},
    {video_keys::FMS_MP_BG,                L"FMS_mp_bg.bk2"},
    {video_keys::FMS_PLAYER_CUSTOMIZATION, L"FMS_player_customization.bk2"},
    {video_keys::FMS_POSTMATCH,            L"FMS_postmatch.bk2"},
    {video_keys::H2_VIGNETTE,              L"H2Vignette.bk2"},
    {video_keys::H3_ODST_VIGNETTE,         L"H3ODSTVignette.bk2"},
    {video_keys::H3_VIGNETTE,              L"H3Vignette.bk2"},
    {video_keys::H4_VIGNETTE,              L"H4Vignette.bk2"},
    {video_keys::HALO_2_ANNIVERSARY,       L"Halo 2 Anniversary.bk2"},
    {video_keys::HALO_CE_ANNIVERSARY,      L"Halo CE Anniversary.bk2"},
    {video_keys::HALO_REACH_VIGNETTE,      L"HaloReachVignette.bk2"},
    {video_keys::HALO_RING_VIGNETTE,       L"HaloRingVignette.bk2"},
    {video_keys::HCE_VIGNETTE,             L"HCEVignette.bk2"},
    {video_keys::MAINMENU_ACE,             L"mainmenu_ace.bk2"},
    {video_keys::MAINMENU_OFF,             L"mainmenu_off.bk2"},
    {video_keys::MYTHIC,                   L"MYTHIC.bk2"},
    {video_keys::NOBLE,                    L"NOBLE.bk2"},
    {video_keys::RAVEN,                    L"RAVEN.bk2"},
    {video_keys::RECLAIMER,                L"RECLAIMER.bk2"},
    {video_keys::RECON,                    L"RECON.bk2"},
    {video_keys::SPARK,                    L"SPARK.bk2"},
    {video_keys::WARTHOG_VIGNETTE,         L"WarthogVignette.bk2"},
    {video_keys::YAPYAP_VIGNETTE,          L"YapyapVignette.bk2"}
};

frozen::unordered_map<video_keys, std::string_view, 38> basenames = {
    {video_keys::ANVIL,                    "anvil.bk2"},
    {video_keys::COVENANT_WAR,             "CovenantWar.bk2"}, 
    {video_keys::ELITE,                    "ELITE.bk2"},
    {video_keys::FLOOD_VIGNETTE,           "FloodVignette.bk2"},
    {video_keys::FMS_CR_BG,                "FMS_cr_bg.bk2"}, 
    {video_keys::FMS_H1_BG,                "FMS_h1_bg.bk2"},
    {video_keys::FMS_H2_BG,                "FMS_h2_bg.bk2"}, 
    {video_keys::FMS_H3_BG,                "FMS_h3_bg.bk2"},
    {video_keys::FMS_H3_ODST_BG,           "FMS_h3odst_bg.bk2"},
    {video_keys::FMS_H4_BG,                "FMS_h4_bg.bk2"}, 
    {video_keys::FMS_HR_BG,                "FMS_hr_bg.bk2"},
    {video_keys::FMS_LOADING,              "FMS_loading.bk2"},
    {video_keys::FMS_LOGO_343_7_1,         "FMS_logo_343_7_1_.bk2"},
    {video_keys::FMS_LOGO_MICROSOFT_7_1,   "FMS_logo_microsoft_7_1_.bk2"},
    {video_keys::FMS_MAINMENU,             "FMS_MainMenu.bk2"},
    {video_keys::FMS_MAINMENU_V2,          "FMS_MainMenu_v2.bk2"},
    {video_keys::FMS_MP_BG,                "FMS_mp_bg.bk2"},
    {video_keys::FMS_PLAYER_CUSTOMIZATION, "FMS_player_customization.bk2"},
    {video_keys::FMS_POSTMATCH,            "FMS_postmatch.bk2"},
    {video_keys::H2_VIGNETTE,              "H2Vignette.bk2"},
    {video_keys::H3_ODST_VIGNETTE,         "H3ODSTVignette.bk2"},
    {video_keys::H3_VIGNETTE,              "H3Vignette.bk2"},
    {video_keys::H4_VIGNETTE,              "H4Vignette.bk2"},
    {video_keys::HALO_2_ANNIVERSARY,       "Halo 2 Anniversary.bk2"},
    {video_keys::HALO_CE_ANNIVERSARY,      "Halo CE Anniversary.bk2"},
    {video_keys::HALO_REACH_VIGNETTE,      "HaloReachVignette.bk2"},
    {video_keys::HALO_RING_VIGNETTE,       "HaloRingVignette.bk2"},
    {video_keys::HCE_VIGNETTE,             "HCEVignette.bk2"},
    {video_keys::MAINMENU_ACE,             "mainmenu_ace.bk2"},
    {video_keys::MAINMENU_OFF,             "mainmenu_off.bk2"},
    {video_keys::MYTHIC,                   "MYTHIC.bk2"},
    {video_keys::NOBLE,                    "NOBLE.bk2"},
    {video_keys::RAVEN,                    "RAVEN.bk2"},
    {video_keys::RECLAIMER,                "RECLAIMER.bk2"},
    {video_keys::RECON,                    "RECON.bk2"},
    {video_keys::SPARK,                    "SPARK.bk2"},
    {video_keys::WARTHOG_VIGNETTE,         "WarthogVignette.bk2"},
    {video_keys::YAPYAP_VIGNETTE,          "YapyapVignette.bk2"}
};


std::vector<std::wstring_view> get_w(video_keys key_flags) {
    std::vector<std::wstring_view> video_basenames;

    for (size_t i = 0; i < 38; ++i) {
        video_keys flag = (video_keys)(1ULL << i);
        if ((key_flags & flag) == flag) {
            video_basenames.push_back(basenames_w.at(flag));
        }
    }

    return video_basenames;
}

std::vector<std::string_view> get(video_keys key_flags) {
    std::vector<std::string_view> video_basenames;

    for (size_t i = 0; i < 38; ++i) {
        video_keys flag = (video_keys)(1ULL << i);
        if ((key_flags & flag) == flag) {
            video_basenames.push_back(basenames.at(flag));
        }
    }

    return video_basenames;
}

}
}

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

#undef BITFLAG
#undef MASK_BITS_UP_TO

} // namespace constants
} // namespace mccinfo