#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <krabs/krabs.hpp> //#include <windows.h>
#include <tlhelp32.h>
#undef NOMINMAX

#include <cstdint>
#pragma warning(push)
#pragma warning(disable : 4068)
#pragma warning(disable : 4996)
#include <cometa.hpp>
#include <cident.h>
#pragma warning(pop)
#include <string>
#include <vector>
#include <fstream>
#include <optional>
#include <filesystem>
#include <span>
namespace mccinfo {
namespace utility {
class atomic_mutex {
  public:
    void lock() {
        while (flag.exchange(true, std::memory_order_relaxed))
            ;
        std::atomic_thread_fence(std::memory_order_acquire);
    }

    void unlock() {
        std::atomic_thread_fence(std::memory_order_release);
        flag.store(false, std::memory_order_relaxed);
    }

  private:
    std::atomic<bool> flag{false};
};

class atomic_guard {
  public:
    atomic_guard(atomic_mutex &mutex) : m_Mutex(mutex) {
        m_Mutex.lock();
    }
    ~atomic_guard() {
        m_Mutex.unlock();
    }

  private:
    atomic_mutex &m_Mutex;
};

template <typename T>
constexpr const char* func_sig() {
    #ifdef _MSC_VER
    return __FUNCSIG__;
    #else
    return __PRETTY_FUNCTION__;
    #endif
}

constexpr auto parse_type(std::string_view sv) {
    auto start = sv.find_last_of('<');
    auto end = sv.find_first_of('>');

    if (start == std::string_view::npos || end == std::string_view::npos || end < start) {
        return sv;
    }

    return sv.substr(start, end - start + 1);
}

constexpr auto remove_namespaces(std::string_view sv) {
    auto last = sv.find_last_of(':');
    if (last == std::string_view::npos) {
        return sv;
    }
    return sv.substr(last + 1, sv.size() - (last + 1) - 1);
}

template <typename T>
constexpr auto make_type_name() {
    return parse_type(func_sig<T>());
}

template <typename T>
constexpr auto make_type_name_minimal() {
    return remove_namespaces(parse_type(func_sig<T>()));
}

template <typename T> struct type_hash {
    static constexpr T type;
    static constexpr std::string_view name{make_type_name<T>()};
    static constexpr std::string_view name_minimal{make_type_name_minimal<T>()};
};

template <typename T>
static constexpr auto type_hash_v = type_hash<T>::value;

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

std::optional<std::string> ConvertWStringToBytes(const std::wstring &wstr) {
    int required_size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()),
                                            nullptr, 0, nullptr, nullptr);
    if (required_size == 0)
        return std::nullopt;

    std::string result(static_cast<size_t>(required_size), '\0');
    int converted = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()),
                                        &result[0], required_size, nullptr, nullptr);
    if (converted == 0)
        return std::nullopt;

    return result;
}

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

std::optional<size_t> GetParentProcessID(size_t pid) {
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

bool IsThreadInProcess(DWORD threadID, DWORD processID) {
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
} // namespace utility
} // namespace mccinfo