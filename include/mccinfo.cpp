#include "mccinfo.h"
namespace mccinfo {
namespace {
std::optional<std::wstring> LookForMCCInVDF(const std::wstring &vdf) {
    try {
        std::ifstream file(vdf);
        auto root = tyti::vdf::read(file);
        for (const auto &libFolder : root.childs) {
            std::filesystem::path folder = libFolder.second->attribs["path"];
            for (const auto &app : libFolder.second->childs) {
                for (const auto &appid : app.second->attribs) {
                    if (appid.first.compare(std::to_string(MCCSteamAppID)) == 0) {
                        return (folder / "\\steamapps\\common\\Halo The Master Chief Collection")
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

std::optional<std::wstring> LookForMCCInSteamApps(const std::wstring &steam_install) {
    std::filesystem::path steamapps = steam_install + L"\\steamapps";
    if (std::filesystem::is_directory(steamapps)) {
        std::filesystem::path vdf = steamapps.generic_wstring() + L"\\libraryfolders.vdf";
        if (std::filesystem::is_regular_file(vdf)) {
            return LookForMCCInVDF(vdf.generic_wstring());
        }
    }
    return std::nullopt;
}

std::optional<std::wstring> LookForMCCInMuiCache(const wil::unique_hkey &hKeyGuard,
                                                 const std::wstring &substr, uint32_t value_count) {
    wchar_t valueName[MAX_PATH];
    DWORD valueNameSize;

    for (DWORD i = 0; i < value_count; ++i) {
        valueNameSize = MAX_PATH;
        LONG result = RegEnumValue(hKeyGuard.get(), i, valueName, &valueNameSize, nullptr, nullptr,
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

// TODO: Replace with managed HANDLE from wil
std::optional<size_t> GetProcessIDFromName(const std::wstring &process_name) {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe;
    size_t pid = 0;
    BOOL hResult;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot)
        return std::nullopt;

    pe.dwSize = sizeof(PROCESSENTRY32);
    hResult = Process32First(hSnapshot, &pe);

    while (hResult) {
        if (wcscmp(process_name.c_str(), pe.szExeFile) == 0) {
            pid = pe.th32ProcessID;
            CloseHandle(hSnapshot);
            return pid;
        }
        hResult = Process32Next(hSnapshot, &pe);
    }
    CloseHandle(hSnapshot);
    return std::nullopt;
}
} // namespace

std::optional<std::wstring> LookForSteamInstallPath(void) {
    try {
        wil::unique_hkey hKeyGuard =
            wil::reg::open_unique_key(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Valve\\Steam");
        return wil::reg::try_get_value_string(hKeyGuard.get(), L"InstallPath");
    } catch (const wil::ResultException &) {
        return std::nullopt;
    }
}

std::optional<std::wstring> LookForMCCSteamInstall(void) {
    auto path = LookForSteamInstallPath();
    if (path.has_value()) {
        return LookForMCCInSteamApps(path.value());
    } else {
        return std::nullopt;
    }
}

std::optional<std::wstring> LookForMCCMicrosoftStoreInstall(void) {
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
std::optional<size_t> LookForMCCProcessID(void) {
    auto windows_pid = GetProcessIDFromName(std::wstring(MicrosoftStoreMCCexe));
    if (windows_pid.has_value()) {
        return windows_pid.value();
    }
    auto steam_pid = GetProcessIDFromName(std::wstring(SteamMCCexe));
    if (steam_pid.has_value()) {
        return steam_pid.value();
    }
    return std::nullopt;
}
} // namespace mccinfo