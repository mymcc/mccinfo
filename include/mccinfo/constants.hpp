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

#define MASK_BITS_IN_RANGE(x, y) \
    (MASK_BITS_UP_TO(y) ^ ((x) > 0 ? MASK_BITS_UP_TO((x) - 1) : 0))

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

inline frozen::unordered_map<video_keys, std::wstring_view, 38> basenames_w = {
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

inline frozen::unordered_map<video_keys, std::string_view, 38> basenames = {
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


inline std::vector<std::wstring_view> get_w(video_keys key_flags) {
    std::vector<std::wstring_view> video_basenames;

    for (size_t i = 0; i < 38; ++i) {
        video_keys flag = (video_keys)(1ULL << i);
        if ((key_flags & flag) == flag) {
            video_basenames.push_back(basenames_w.at(flag));
        }
    }

    return video_basenames;
}

inline std::vector<std::string_view> get(video_keys key_flags) {
    std::vector<std::string_view> video_basenames;

    for (size_t i = 0; i < 38; ++i) {
        video_keys flag = (video_keys)(1ULL << i);
        if ((key_flags & flag) == flag) {
            video_basenames.push_back(basenames.at(flag));
        }
    }

    return video_basenames;
}

} // menu
} // background_videos

namespace localization_binaries {

enum lang_keys : uint64_t {
    CS  = BITFLAG(0),
    CT  = BITFLAG(1),
    DA  = BITFLAG(2),
    DBG = BITFLAG(3),
    DE  = BITFLAG(4),
    DU  = BITFLAG(5),
    EN  = BITFLAG(6),
    FI  = BITFLAG(7),
    FR  = BITFLAG(8),
    IT  = BITFLAG(9),
    JP  = BITFLAG(10),
    KO  = BITFLAG(11),
    NO  = BITFLAG(12),
    PB  = BITFLAG(13),
    PO  = BITFLAG(14),
    PR  = BITFLAG(15),
    RU  = BITFLAG(16),
    SP  = BITFLAG(17),
    SU  = BITFLAG(18),
    ALL = MASK_BITS_UP_TO(18),
};

enum game_keys : uint64_t {
    GLOBAL      = BITFLAG(0),
    HALO1       = BITFLAG(1),
    HALO2       = BITFLAG(2),
    HALO2A      = BITFLAG(3),
    HALO3       = BITFLAG(4),
    HALO3ODST   = BITFLAG(5),
    HALO4       = BITFLAG(6),
    HALOREACH   = BITFLAG(7),
    DEBUG       = BITFLAG(8),
    BLNK_GLOBAL = BITFLAG(9),
    ID_GLOBAL   = BITFLAG(10),
    LBL_GLOBAL  = BITFLAG(11),
    MAX_GLOBAL  = BITFLAG(12),
    ALL_GAMES   = MASK_BITS_IN_RANGE(1, 7),
};
} // localization_binaries
} // constants
} // mccinfo

namespace frozen {

template <> struct elsa<mccinfo::constants::localization_binaries::lang_keys> : elsa<uint64_t> {};
template <> struct elsa<mccinfo::constants::localization_binaries::game_keys> : elsa<uint64_t> {};

} // frozen

namespace mccinfo{
namespace constants{
namespace localization_binaries {

inline frozen::unordered_map<lang_keys, std::wstring_view, 19> lang_prefixes_w = {
    {lang_keys::CS,  L"CS_"},
    {lang_keys::CT,  L"CT_"},
    {lang_keys::DA,  L"DA_"},
    {lang_keys::DBG, L"DBG_"},
    {lang_keys::DE,  L"DE_"},
    {lang_keys::DU,  L"DU_"},
    {lang_keys::EN,  L"EN_"},
    {lang_keys::FI,  L"FI_"},
    {lang_keys::FR,  L"FR_"},
    {lang_keys::IT,  L"IT_"},
    {lang_keys::JP,  L"JP_"},
    {lang_keys::KO,  L"KO_"},
    {lang_keys::NO,  L"NO_"},
    {lang_keys::PB,  L"PB_"},
    {lang_keys::PO,  L"PO_"},
    {lang_keys::PR,  L"PR_"},
    {lang_keys::RU,  L"RU_"},
    {lang_keys::SP,  L"SP_"},
    {lang_keys::SU,  L"SU_"}
};

inline frozen::unordered_map<lang_keys, std::string_view, 19> lang_prefixes = {
    {lang_keys::CS,  "CS_"},
    {lang_keys::CT,  "CT_"},
    {lang_keys::DA,  "DA_"},
    {lang_keys::DBG, "DBG_"},
    {lang_keys::DE,  "DE_"},
    {lang_keys::DU,  "DU_"},
    {lang_keys::EN,  "EN_"},
    {lang_keys::FI,  "FI_"},
    {lang_keys::FR,  "FR_"},
    {lang_keys::IT,  "IT_"},
    {lang_keys::JP,  "JP_"},
    {lang_keys::KO,  "KO_"},
    {lang_keys::NO,  "NO_"},
    {lang_keys::PB,  "PB_"},
    {lang_keys::PO,  "PO_"},
    {lang_keys::PR,  "PR_"},
    {lang_keys::RU,  "RU_"},
    {lang_keys::SP,  "SP_"},
    {lang_keys::SU,  "SU_"}
};

inline frozen::unordered_map<game_keys, std::wstring_view, 13> game_suffixes_w = {
    {game_keys::GLOBAL,      L"Global"},
    {game_keys::HALO1,       L"Halo1"},
    {game_keys::HALO2,       L"Halo2"},
    {game_keys::HALO2A,      L"Halo2A"}, 
    {game_keys::HALO3,       L"Halo3"}, 
    {game_keys::HALO3ODST,   L"Halo3ODST"},
    {game_keys::HALO4,       L"Halo4"},   
    {game_keys::HALOREACH,   L"HaloReach"},
    {game_keys::DEBUG,       L"Debug"},   
    {game_keys::BLNK_GLOBAL, L"BLNK_Global"},
    {game_keys::ID_GLOBAL,   L"ID_Global"}, 
    {game_keys::LBL_GLOBAL,  L"LBL_Global"},
    {game_keys::MAX_GLOBAL,  L"MAX_Global"}
};

inline frozen::unordered_map<game_keys, std::string_view, 13> game_suffixes = {
    {game_keys::GLOBAL,      "Global"},
    {game_keys::HALO1,       "Halo1"},
    {game_keys::HALO2,       "Halo2"},
    {game_keys::HALO2A,      "Halo2A"}, 
    {game_keys::HALO3,       "Halo3"}, 
    {game_keys::HALO3ODST,   "Halo3ODST"},
    {game_keys::HALO4,       "Halo4"},   
    {game_keys::HALOREACH,   "HaloReach"},
    {game_keys::DEBUG,       "Debug"},   
    {game_keys::BLNK_GLOBAL, "BLNK_Global"},
    {game_keys::ID_GLOBAL,   "ID_Global"}, 
    {game_keys::LBL_GLOBAL,  "LBL_Global"},
    {game_keys::MAX_GLOBAL,  "MAX_Global"}
};

inline std::vector<std::wstring> get_w(lang_keys key_flags, uint64_t game_flags) {

    std::vector<std::wstring_view> lang_prefixes;
    for (size_t i = 0; i < 19; ++i) {
        lang_keys flag = (lang_keys)(1ULL << i);
        if ((key_flags & flag) == flag) {
            lang_prefixes.push_back(lang_prefixes_w.at(flag));
        }
    }

    std::vector<std::wstring_view> game_suffixes;
    for (size_t i = 0; i < 13; ++i) {
        game_keys flag = (game_keys)(1ULL << i);
        if ((game_flags & flag) == flag) {
            game_suffixes.push_back(game_suffixes_w.at(flag));
        }
    }

    std::vector<std::wstring> lang_binary_basenames;
    if ((lang_prefixes.size() > 0) && (game_suffixes.size() > 0)) {
        for (const auto &prefix : lang_prefixes) {
            std::wstring basename = std::wstring(prefix);
            for (const auto& suffix : game_suffixes) {
                lang_binary_basenames.push_back(basename + std::wstring(suffix));
            }
        }
    }

    return lang_binary_basenames;
}

inline std::vector<std::string> get(lang_keys key_flags, game_keys game_flags) {
    std::vector<std::string> lang_binary_basenames;

    std::string_view lang_prefix;
    for (size_t i = 0; i < 19; ++i) {
        lang_keys flag = (lang_keys)(1ULL << i);
        if ((key_flags & flag) == flag) {
            lang_prefix = lang_prefixes.at(flag);
        }
    }

    std::string_view game_suffix;
    for (size_t i = 0; i < 13; ++i) {
        game_keys flag = (game_keys)(1ULL << i);
        if ((key_flags & flag) == flag) {
            game_suffix = game_suffixes.at(flag);
        }
    }

    if ((lang_prefix.size() > 0) && (game_suffix.size() > 0)) {
        lang_binary_basenames.push_back(std::string(lang_prefix) + std::string(game_suffix));
    }

    return lang_binary_basenames;
}

} // namespace localization_binaries

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
/**
 * @brief The number of bytes Halo: The Master Chief Collection&trade; reads from its .pak (asset package) at a time during runtime.
 */
inline constexpr size_t font_package_read_size = 49152;
/**
 * @brief The number of bytes Halo: The Master Chief Collection&trade; reads from its sounds_dialog.fsb (Halo 1 asset package) at the start of a Halo 1 match.
 */
inline constexpr size_t halo1_initial_fsb_read_size = 131072;
#undef BITFLAG
#undef MASK_BITS_UP_TO

} // namespace constants
} // namespace mccinfo