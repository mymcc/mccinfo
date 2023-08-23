#include "mccinfo.h"
namespace mccinfo {
namespace {
std::optional<std::vector<char>> SlurpFile(const std::filesystem::path path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(file_size);
    if (file.read(buffer.data(), file_size))
        return buffer;
    else
        return std::nullopt;
}
std::optional<std::wstring> LookForMCCInVDF(const std::wstring &vdf) {
    try {
        std::ifstream file(vdf);
        auto root = tyti::vdf::read(file);
        for (const auto &libFolder : root.childs) {
            std::filesystem::path folder = libFolder.second->attribs["path"];
            for (const auto &app : libFolder.second->childs) {
                for (const auto &appid : app.second->attribs) {
                    if (appid.first.compare(std::to_string(MCCSteamAppID)) == 0) {
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

std::optional<std::filesystem::path> ExpandPath(const std::filesystem::path &path) {
    std::wstring dst;
    dst.resize(MAX_PATH);
    DWORD ret = ::ExpandEnvironmentStringsW(path.wstring().c_str(), dst.data(), MAX_PATH);
    if (ret == 0)
        return std::nullopt;
    if (!std::filesystem::exists(dst))
        return std::nullopt;
    return std::filesystem::absolute(dst);
}
} // namespace
std::wostream& operator<<(std::wostream& os, const MCCInstallInfo& ii) {
    constexpr uint8_t align = 18;

    os << L"MCC Installation Info:" << std::endl
        << std::left << std::setw(align) << L"Kind: " << StoreVersionToWString.at(ii.Kind) << L" ("
        << ii.BuildVersion << L") " << ii.BuildTag << std::endl
        << std::left << std::setw(align) << L"RootPath: " << ii.RootPath << std::endl
        << std::left << std::setw(align) << L"Installed Games: ";
    for (size_t i = 0; i < ii.InstalledGames.size(); ++i) {
        os << ii.InstalledGames[i];
        if (i != ii.InstalledGames.size() - 1) {
            os << L", ";
        }
    }
    return os;
}
std::optional<MCCInstallInfo> LookForInstallInfoImpl (
    const std::wstring& install_path, StoreVersion store_version = StoreVersion::None) {
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
                    for (const auto& game : installed.value()) {
                        ii.InstalledGames.push_back(std::filesystem::path(game).stem());
                    }
                }
                return ii;
            }
        }
    }
    return std::nullopt;
}
std::optional<std::wstring> ConvertBytesToWString(const std::string &bytes) {
    int required_size =
        MultiByteToWideChar(CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
    if (required_size == 0)
        return std::nullopt;

    std::wstring result(static_cast<size_t>(required_size), L'\0');
    int converted = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()),
                                        &result[0], required_size);
    if (converted == 0)
        return std::nullopt;

    return result;
}

std::optional<std::wstring> LookForSteamInstallPath(void) {
    try {
        wil::unique_hkey hKeyGuard =
            wil::reg::open_unique_key(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Valve\\Steam");
        return wil::reg::try_get_value_string(hKeyGuard.get(), L"InstallPath");
    } catch (const wil::ResultException &) {
        return std::nullopt;
    }
}

std::optional<std::wstring> LookForMCCSteamInstallPath(void) {
    auto path = LookForSteamInstallPath();
    if (path.has_value()) {
        return LookForMCCInSteamApps(path.value());
    } else {
        return std::nullopt;
    }
}

std::optional<std::wstring> LookForMCCMicrosoftStoreInstallPath(void) {
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

std::optional<std::wstring> LookForMCCTempPath(void) {
    auto temp_root = ExpandPath(MCCTempPath);
    if (temp_root.has_value()) {
        return temp_root.value();
    } else {
        return std::nullopt;
    }
}

std::optional<std::wstring> GetFileVersion(const std::wstring &path) {
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

std::optional<std::wstring> LookForMCCBuildTag(const std::wstring &install_path) {
    std::filesystem::path path(install_path);
    if (std::filesystem::exists(install_path)) {
        auto build_tag_file = path / "build_tag.txt";
        if (std::filesystem::exists(build_tag_file)) {
            auto tag_file_read = SlurpFile(build_tag_file);
            if (tag_file_read.has_value()) {
                std::istringstream iss(
                    std::string(tag_file_read.value().data(), tag_file_read.value().size()));

                std::string line;
                while (std::getline(iss, line)) {
                    break; // read first line
                }

                auto ws = ConvertBytesToWString(line);
                return ws;
            }
        }
    }
    return std::nullopt;
}

std::optional<std::vector<std::wstring>> LookForInstalledGameDLLs(
    const std::wstring &install_path) {
    std::filesystem::path path(install_path);
    if (std::filesystem::exists(install_path)) {
        std::vector<std::wstring> InstalledGameDLLs;
        for (const auto &game : GameBasenames) {
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

#pragma comment(lib, "Version.lib")
std::optional<std::wstring> LookForMCCBuildVersion(const std::wstring &install_path) {
    std::filesystem::path path(install_path);
    std::array<std::wstring_view, 2> exes = {SteamMCCexe, MicrosoftStoreMCCexe};

    for (const auto &exe : exes) {
        auto exe_path = (path / MCCexeRelativePath / exe).make_preferred();
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
std::optional<StoreVersion> LookForMCCKind(const std::wstring &install_path) {
    std::filesystem::path path(install_path);
    std::array<std::wstring_view, 2> exes = {SteamMCCexe, MicrosoftStoreMCCexe};

    for (const auto &exe : exes) {
        auto exe_path = (path / MCCexeRelativePath / exe).make_preferred();
        if (std::filesystem::exists(exe_path)) {
            if (exe == SteamMCCexe)
                return StoreVersion::Steam;
            else
                return StoreVersion::MicrosoftStore;
        }
    }
    return std::nullopt;
}
std::optional<MCCInstallInfo> LookForInstallInfo(const std::wstring &install_path) {
    return LookForInstallInfoImpl(install_path);
}
std::optional<MCCInstallInfo> LookForInstallInfo(StoreVersion store_version) {
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
std::optional<MCCInstallInfo> LookForSteamInstallInfo(void) {
    return LookForInstallInfo(StoreVersion::Steam);
}
std::optional<MCCInstallInfo> LookForMicrosoftStoreInstallInfo(void) {
    return LookForInstallInfo(StoreVersion::MicrosoftStore);
}
} // namespace mccinfo