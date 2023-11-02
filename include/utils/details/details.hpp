#pragma once
#define NOMINMAX
#include <windows.h>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <optional>
#include <filesystem>

namespace mccinfo {
namespace utils {
namespace details {
inline std::optional<std::vector<char>> SlurpFile(const std::filesystem::path path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(file_size);
    if (file.read(buffer.data(), file_size))
        return buffer;
    else
        return std::nullopt;
}
inline std::optional<std::wstring> LookForMCCInVDF(const std::wstring& vdf) {
    try {
        std::ifstream file(vdf);
        auto root = tyti::vdf::read(file);
        for (const auto& libFolder : root.childs) {
            std::filesystem::path folder = libFolder.second->attribs["path"];
            for (const auto& app : libFolder.second->childs) {
                for (const auto& appid : app.second->attribs) {
                    if (appid.first.compare(std::to_string(MCCSteamAppID)) == 0) {
                        return (folder / "steamapps\\common\\Halo The Master Chief Collection")
                            .make_preferred();
                    }
                }
            }
        }
    }
    catch (const std::ios_base::failure& e) {
        std::cerr << "File operation failed: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "An unknown error occurred." << std::endl;
    }
    return std::nullopt;
}

inline std::optional<std::wstring> LookForMCCInSteamApps(const std::wstring& steam_install) {
    std::filesystem::path steamapps = steam_install + L"\\steamapps";
    if (std::filesystem::is_directory(steamapps)) {
        std::filesystem::path vdf = steamapps.generic_wstring() + L"\\libraryfolders.vdf";
        if (std::filesystem::is_regular_file(vdf)) {
            return LookForMCCInVDF(vdf.generic_wstring());
        }
    }
    return std::nullopt;
}

inline std::optional<std::wstring> LookForMCCInMuiCache(const wil::unique_hkey& hKeyGuard,
    const std::wstring& substr, uint32_t value_count) {
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
        }
        else {
            std::cerr << "Failed to enumerate registry value. Error code: " << result << std::endl;
            break;
        }
    }
    return std::nullopt;
}

// TODO: Replace with managed HANDLE from wil
inline std::optional<size_t> GetProcessIDFromName(const std::wstring& process_name) {
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

inline std::optional<size_t> GetParentProcessID(size_t pid) {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe;
    size_t ppid = 0;
    BOOL hResult;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot)
        return std::nullopt;

    pe.dwSize = sizeof(PROCESSENTRY32);
    hResult = Process32First(hSnapshot, &pe);

    while (hResult) {
        if (pid == pe.th32ProcessID) {
            ppid = pe.th32ParentProcessID;
            CloseHandle(hSnapshot);
            return ppid;
        }
        hResult = Process32Next(hSnapshot, &pe);
    }
    CloseHandle(hSnapshot);
    return std::nullopt;
}

inline bool IsThreadInProcess(DWORD threadID, DWORD processID) {
    // Take a snapshot of all running threads
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        return false;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    // Get the information about the first thread
    if (!Thread32First(hThreadSnap, &te32)) {
        CloseHandle(hThreadSnap);
        return false;
    }

    // Now walk the thread list of the system
    do {
        if (te32.th32OwnerProcessID == processID && te32.th32ThreadID == threadID) {
            // Found a thread with the given thread ID that belongs to the process with the given
            // process ID
            CloseHandle(hThreadSnap);
            return true;
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);
    return false;
}

inline std::optional<std::filesystem::path> ExpandPath(const std::filesystem::path& path) {
    std::wstring dst;
    dst.resize(MAX_PATH);
    DWORD ret = ::ExpandEnvironmentStringsW(path.wstring().c_str(), dst.data(), MAX_PATH);
    if (ret == 0)
        return std::nullopt;
    if (!std::filesystem::exists(dst))
        return std::nullopt;
    return std::filesystem::absolute(dst);
}
}
}
}