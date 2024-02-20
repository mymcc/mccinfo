#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
//#include <windows.h>
#include <wil/filesystem.h>
#include <tlhelp32.h>
#include <wil/registry.h>
#undef NOMINMAX

#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <optional>
#include <filesystem>

#pragma warning(push)
#pragma warning(disable : 4996)
#include <vdf_parser.hpp>
#pragma warning(pop)

#include "utility.hpp"
#include "constants.hpp"

namespace mccinfo {
namespace query {
enum class StoreVersion { None, Steam, MicrosoftStore };

const std::unordered_map<StoreVersion, std::wstring_view> StoreVersionToWString{
    {StoreVersion::None, L"None"},
    {StoreVersion::Steam, L"Steam"},
    {StoreVersion::MicrosoftStore, L"Microsoft Store"}};

struct MCCInstallInfo {
  public:
    StoreVersion Kind{StoreVersion::None};
    std::wstring RootPath;
    std::wstring BuildTag;
    std::wstring BuildVersion;

    std::vector<std::wstring> InstalledGames;

    friend std::wostream &operator<<(std::wostream &os, const MCCInstallInfo &ii);
    friend std::optional<MCCInstallInfo> LookForInstallInfoImpl(const std::wstring &install_path,
                                                                StoreVersion store_version);

    MCCInstallInfo() {
    }
  private:
};

inline std::wostream &operator<<(std::wostream &os, const MCCInstallInfo &ii) {
    constexpr uint8_t align = 18;

    os << L"\tMCC Installation Info:" << std::endl
       << std::left << std::setw(align) << L"\t\tKind: " << StoreVersionToWString.at(ii.Kind) << L" ("
       << ii.BuildVersion << L") " << ii.BuildTag << std::endl
       << std::left << std::setw(align) << L"\t\tRootPath: " << ii.RootPath << std::endl
       << std::left << std::setw(align) << L"\t\tInstalled Games: ";
    for (size_t i = 0; i < ii.InstalledGames.size(); ++i) {
        os << ii.InstalledGames[i];
        if (i != ii.InstalledGames.size() - 1) {
            os << L", ";
        }
    }
    return os;
}

inline std::optional<StoreVersion> LookForMCCKind(const std::wstring &install_path) {
    std::filesystem::path path(install_path);
    std::array<std::wstring_view, 2> exes = {constants::mcc_steam_exe_w, constants::mcc_msstore_exe_w};

    for (const auto &exe : exes) {
        auto exe_path = (path / constants::mcc_relative_path_to_exe_w / exe).make_preferred();
        if (std::filesystem::exists(exe_path)) {
            if (exe == constants::mcc_steam_exe_w)
                return StoreVersion::Steam;
            else
                return StoreVersion::MicrosoftStore;
        }
    }
    return std::nullopt;
}

inline std::optional<std::vector<std::wstring>> LookForInstalledGameDLLs(
    const std::wstring &install_path) {
    std::filesystem::path path(install_path);
    if (std::filesystem::exists(install_path)) {
        std::vector<std::wstring> InstalledGameDLLs;
        for (const auto &game : constants::mcc_game_basenames_w) {
            auto dll_path = path / game / (std::wstring(game) + L".dll");
            if (std::filesystem::exists(dll_path)) {
                InstalledGameDLLs.push_back(dll_path.generic_wstring());
            }
        }
        if (InstalledGameDLLs.size() > 0)
            return InstalledGameDLLs;
    }
    return std::nullopt;
}

inline std::optional<std::wstring> GetFileVersion(const std::wstring &path) {
    std::filesystem::path file_path(path);

    if (std::filesystem::exists(file_path)) {
        DWORD info_size = GetFileVersionInfoSizeW(file_path.generic_wstring().c_str(), NULL);
        std::vector<wchar_t> file_info(info_size);
        BOOL ret = GetFileVersionInfoW(file_path.generic_wstring().c_str(), NULL, info_size,
                                       file_info.data());

        if (ret != 0) {
            UINT size = 0;
            VS_FIXEDFILEINFO *pFileInfo;
            ret = VerQueryValueW((void *)file_info.data(), L"\\", (LPVOID *)&pFileInfo, &size);

            if (ret != 0) {
                std::wstringstream wss;

                wss << ((pFileInfo->dwFileVersionMS >> 16) & 0xffff) << '.'
                    << ((pFileInfo->dwFileVersionMS) & 0xffff) << '.'
                    << ((pFileInfo->dwFileVersionLS >> 16) & 0xffff) << '.'
                    << ((pFileInfo->dwFileVersionLS) & 0xffff);

                return wss.str();
            }
        }
    }
    return std::nullopt;
}

inline std::optional<std::wstring> LookForMCCBuildVersion(const std::wstring &install_path) {
    std::filesystem::path path(install_path);
    std::array<std::wstring_view, 2> exes = {constants::mcc_steam_exe_w, constants::mcc_msstore_exe_w};

    for (const auto &exe : exes) {
        auto exe_path = (path / constants::mcc_relative_path_to_exe_w / exe).make_preferred();
        if (std::filesystem::exists(exe_path)) {
            auto fv = GetFileVersion(path.generic_wstring());
            if (fv.has_value()) {
                return fv;
            } else {
                // search through available game .dlls for version (Likely for MS Store)
                auto game_dlls = LookForInstalledGameDLLs(path);
                if (game_dlls.has_value()) {
                    for (const auto &dll : game_dlls.value()) {
                        auto fv_dll = GetFileVersion(dll);
                        if (fv_dll.has_value()) {
                            return fv_dll;
                        }
                    }
                }
            }
        }
    }
    return std::nullopt;
}

inline std::optional<std::wstring> LookForMCCBuildTag(const std::wstring &install_path) {
    std::filesystem::path path(install_path);
    if (std::filesystem::exists(install_path)) {
        auto build_tag_file = path / "build_tag.txt";
        if (std::filesystem::exists(build_tag_file)) {
            auto tag_file_read = utility::SlurpFile(build_tag_file);
            if (tag_file_read.has_value()) {
                std::istringstream iss(
                    std::string(tag_file_read.value().data(), tag_file_read.value().size()));

                std::string line;
                while (std::getline(iss, line)) {
                    break; // read first line
                }

                auto ws = utility::ConvertBytesToWString(line);
                return ws;
            }
        }
    }
    return std::nullopt;
}

inline std::optional<MCCInstallInfo> LookForInstallInfoImpl(
    const std::wstring &install_path, StoreVersion store_version = StoreVersion::None) {
    MCCInstallInfo ii;

    if (std::filesystem::exists(install_path)) {
        ii.RootPath = install_path;

        auto kind = LookForMCCKind(install_path);
        if (kind.has_value()) {
            if ((store_version != StoreVersion::None) && (kind.value() != store_version))
                return std::nullopt;

            ii.Kind = kind.value();

            auto bv = LookForMCCBuildVersion(install_path);
            auto bt = LookForMCCBuildTag(install_path);

            if (bv.has_value() && bt.has_value()) {
                ii.BuildVersion = bv.value();
                ii.BuildTag = bt.value();

                auto installed = LookForInstalledGameDLLs(install_path);
                if (installed.has_value()) {
                    for (const auto &game : installed.value()) {
                        ii.InstalledGames.push_back(std::filesystem::path(game).stem());
                    }
                }
                return ii;
            }
        }
    }
    return std::nullopt;
}

inline std::optional<std::wstring> LookForMCCInVDF(const std::wstring &vdf) {
    try {
        std::ifstream file(vdf);
        auto root = tyti::vdf::read(file);
        for (const auto &libFolder : root.childs) {
            std::filesystem::path folder = libFolder.second->attribs["path"];
            for (const auto &app : libFolder.second->childs) {
                for (const auto &appid : app.second->attribs) {
                    if (appid.first.compare(std::to_string(constants::mcc_steam_app_id)) == 0) {
                        return (folder / "steamapps\\common\\Halo The Master Chief Collection")
                            .make_preferred();
                    }
                }
            }
        }
    } catch (const std::ios_base::failure &e) {
        std::cerr << "File operation failed: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "An unknown error occurred." << std::endl;
    }
    return std::nullopt;
}

inline std::optional<std::wstring> LookForMCCInSteamApps(const std::wstring &steam_install) {
    std::filesystem::path steamapps = steam_install + L"\\steamapps";
    if (std::filesystem::is_directory(steamapps)) {
        std::filesystem::path vdf = steamapps.generic_wstring() + L"\\libraryfolders.vdf";
        if (std::filesystem::is_regular_file(vdf)) {
            return LookForMCCInVDF(vdf.generic_wstring());
        }
    }
    return std::nullopt;
}

inline std::optional<std::wstring> LookForMCCInMuiCache(const wil::unique_hkey &hKeyGuard,
                                                 const std::wstring &substr, uint32_t value_count) {
    wchar_t valueName[MAX_PATH];
    DWORD valueNameSize;

    for (DWORD i = 0; i < value_count; ++i) {
        valueNameSize = MAX_PATH;
        LONG result = RegEnumValueW(hKeyGuard.get(), i, valueName, &valueNameSize, nullptr, nullptr,
                                    nullptr, nullptr);
        if (result == ERROR_SUCCESS) {
            std::wstring w(valueName, MAX_PATH);
            size_t pos = w.find(substr.c_str());
            if (pos != std::wstring::npos) {
                return w.substr(0, pos) + L"content";
            }
        } else {
            std::cerr << "Failed to enumerate registry value. Error code: " << result << std::endl;
            break;
        }
    }
    return std::nullopt;
}
/**
 * @brief Retrieves the installation path of Steam&trade;.
 *
 * Attempts to locate the Steam&trade; installation directory, which is typically found at
 * <b>C:\\Program Files (x86)\\Steam</b>. If the installation path cannot be determined,
 * the returned optional will be empty.
 *
 * @return An optional containing the installation path of Steam&trade; if found; otherwise, an
 * empty optional.
 */
inline std::optional<std::wstring> LookForSteamInstallPath(void) {
    try {
        wil::unique_hkey hKeyGuard =
            wil::reg::open_unique_key(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Valve\\Steam");
        return wil::reg::try_get_value_string(hKeyGuard.get(), L"InstallPath");
    } catch (const wil::ResultException &) {
        return std::nullopt;
    }
}
/**
 * @brief Retrieves the installation path of the Steam&trade; version of Halo: The Master Chief
 * Collection&trade;.
 *
 * Attempts to locate the installation directory of the Steam&trade; version of Halo: The Master
 * Chief Collection&trade;, which is typically found at <b>C:\\Program Files
 * (x86)\\Steam\\steamapps\\common\\Halo The Master Chief Collection\\</b>. If the installation path
 * cannot be determined, the returned optional will be empty.
 *
 * @return An optional containing the installation path of Steam&trade; if found; otherwise, an
 * empty optional.
 */
inline std::optional<std::wstring> LookForMCCSteamInstallPath(void) {
    auto path = LookForSteamInstallPath();
    if (path.has_value()) {
        return LookForMCCInSteamApps(path.value());
    } else {
        return std::nullopt;
    }
}
/**
 * @brief Retrieves the installation path of the Microsoft Store&trade; version of Halo: The Master
 * Chief Collection&trade;.
 *
 * Attempts to locate the installation directory of the Microsoft Store&trade; version of Halo: The
 * Master Chief Collection&trade;, which is typically found at <b>C:\\XboxGames\\Halo The Master
 * Chief Collection\\content\\</b>. If the installation path cannot be determined, the returned
 * optional will be empty.
 *
 * @return An optional containing the installation path of Microsoft Store&trade; if found;
 * otherwise, an empty optional.
 */
inline std::optional<std::wstring> LookForMCCMicrosoftStoreInstallPath(void) {
    try {
        wil::unique_hkey hKeyGuard = wil::reg::open_unique_key(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache");

        uint32_t value_count = wil::reg::get_child_value_count(hKeyGuard.get());

        return LookForMCCInMuiCache(
            hKeyGuard, L"content\\mcc\\binaries\\win64\\mccwinstore-win64-shipping.exe",
            value_count);
    } catch (const wil::ResultException &e) {
        std::cerr << "A Windows Implementation Library Exception occurred: " << e.what()
                  << std::endl;
        return std::nullopt;
    }
}
/**
 * @brief Retrieves the process id of the first found instance of Halo: The Master
 * Chief Collection&trade;.
 *
 * Attempts to find the process id of a running instance Halo: The Master Chief Collection&trade;,
 * with no distinction on whether or not the instance is hosting an Easy Anti-Cheat client. As far
 * as instance version precedence, this function first checks for Microsoft Store instances, then
 * checks for Steam instances. If no instances are found, then the optional will be empty.
 *
 * @return An optional containing the process id of the first found instance of Halo: The Master
 * Chief Collection&trade; if one is found; otherwise, an empty optional.
 */
inline std::optional<size_t> LookForMCCProcessID(void) {
    auto windows_pid = utility::GetProcessIDFromName(std::wstring(constants::mcc_msstore_exe_w));
    if (windows_pid.has_value()) {
        return windows_pid.value();
    }
    auto steam_pid = utility::GetProcessIDFromName(std::wstring(constants::mcc_steam_exe_w));
    if (steam_pid.has_value()) {
        return steam_pid.value();
    }
    return std::nullopt;
}

inline std::optional<std::wstring> LookForMCCTempPath(void) {
    auto temp_root = utility::ExpandPath(constants::mcc_system_temp_path_w);
    if (temp_root.has_value()) {
        return temp_root.value();
    } else {
        return std::nullopt;
    }
}

inline std::optional<MCCInstallInfo> LookForInstallInfo(const std::wstring &install_path) {
    return LookForInstallInfoImpl(install_path);
}

inline std::optional<MCCInstallInfo> LookForInstallInfo(StoreVersion store_version) {
    std::optional<std::filesystem::path> install_path;
    if (store_version == StoreVersion::Steam)
        install_path = LookForMCCSteamInstallPath();
    else if (store_version == StoreVersion::MicrosoftStore)
        install_path = LookForMCCMicrosoftStoreInstallPath();
    else
        return std::nullopt;

    if (install_path.has_value())
        return LookForInstallInfoImpl(install_path.value(), store_version);
    else
        return std::nullopt;
}

inline std::optional<MCCInstallInfo> LookForSteamInstallInfo(void) {
    return LookForInstallInfo(StoreVersion::Steam);
}

inline std::optional<MCCInstallInfo> LookForMicrosoftStoreInstallInfo(void) {
    return LookForInstallInfo(StoreVersion::MicrosoftStore);
}

inline std::optional<HWND> LookForMCCWindowHandle(void) {
    HWND hwnd = FindWindowW(L"UnrealWindow", L"Halo: The Master Chief Collection  ");
    if (hwnd != NULL)
        return hwnd;
    else
        return std::nullopt;
}
} // namespace query
} // namespace mccinfo