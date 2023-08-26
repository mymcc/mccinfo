#include "mccinfo.h"
#include "mccfsm.h"

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
            // Found a thread with the given thread ID that belongs to the process with the given process ID
            CloseHandle(hThreadSnap);
            return true;
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);
    return false;
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

std::optional<std::string> ConvertWStringToBytes(const std::wstring& wstr) {
    int required_size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    if (required_size == 0)
        return std::nullopt;

    std::string result(static_cast<size_t>(required_size), '\0');
    int converted = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), &result[0], required_size, nullptr, nullptr);
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

constexpr std::array<size_t, 4> name_events {
    0, 32, 35, 36
};

constexpr std::array<size_t, 5> events {
    69, 70, 71, 74, 75
};

const std::unordered_map<size_t, std::wstring_view> eventToWstring {
    {69, L"SetInfo"}, { 70, L"Delete" }, { 71, L"Rename" }, { 74, L"QueryInfo" }, { 75, L"FSControl" }
};

std::unordered_map<uint32_t*, std::wstring> FileKeyMap;

constexpr std::array<uint32_t, 5> process_events {
    1, 2, 3, 4, 11
};

krabs::event_filter MakeProcessFilter(void) {
    static std::array<krabs::predicates::opcode_is, 5> opcode_predicates {
        krabs::predicates::opcode_is(process_events[0]),
        krabs::predicates::opcode_is(process_events[1]),
        krabs::predicates::opcode_is(process_events[2]),
        krabs::predicates::opcode_is(process_events[3]),
        krabs::predicates::opcode_is(process_events[4]),
    };

    // This isn't sufficient at filtering:
    // Process_Terminate (opcode: 11, Event ID: 0, Event Version(2))
    // just before Process_End ... bug in etw? - Stehfyn 8/24/23
    static krabs::predicates::version_is version_is(3);
    static krabs::event_filter filter {
        krabs::predicates::and_filter(
            krabs::predicates::any_of({
                &opcode_predicates[0],
                &opcode_predicates[1],
                &opcode_predicates[2],
                &opcode_predicates[3],
                }),
            krabs::predicates::all_of({ 
                &version_is,
                })
        )
    };
    return filter;
}
krabs::event_filter MakeProcessFilter2(void) {
    auto start = krabs::predicates::opcode_is(1);
    auto name = krabs::predicates::property_equals(L"ImageFileName", std::wstring(L"mcclauncher.exe"));

    krabs::event_filter filter {
        krabs::predicates::all_of({
                &start,
                &name
            })
    };

    return filter;
}
#define PRINT_LIMIT 3 // only print a few events for brevity
void process_rundown_callback(const EVENT_RECORD& record, const krabs::trace_context& trace_context);
void file_rundown_callback(const EVENT_RECORD& record, const krabs::trace_context& trace_context);
void hwconfig_callback(const EVENT_RECORD& record, const krabs::trace_context& trace_context);

bool StartETW(void)
{
    
// we will make different filters, attach different callbacks that dispatch their respective event.
// like a successful predicate filter should elicit next_predicate_in_seq
// but a process end for mcc should elicit terminate, but only if its a pid we recognized to be on and were tracking



    fsm_handle::start();
    krabs::kernel_trace trace(L"kernel_trace");
    fsm_kernel_process_provider process_provider;

    fsm_handle::dispatch(SequenceStart(&process_provider));

    trace.enable(process_provider);

    //krabs::kernel::disk_file_io_provider file_io_provider;
    //file_io_provider.add_on_event_callback(file_rundown_callback);
    ////trace.enable(file_io_provider);
    //
    //krabs::kernel_provider hwconfig_provider(0, krabs::guids::event_trace_config);
    //hwconfig_provider.add_on_event_callback(hwconfig_callback);
    //trace.enable(hwconfig_provider);

    //krabs::kernel_trace trace2(L"kernel_trace2");
    //krabs::kernel::disk_file_io_provider

    std::cout << " - starting trace" << std::endl;

    std::thread thread([&trace]() { trace.start(); });

    // We will wait for all start events to be processed.
    // By default ETW buffers are flush when full, or every second otherwise
    Sleep(60000);

    std::cout << std::endl << " - stopping trace" << std::endl;
    trace.stop();
    thread.join();
    return true;
}
void process_rundown_callback(const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
    krabs::schema schema(record, trace_context.schema_locator);
    krabs::parser parser(schema);
    if (schema.event_opcode() != 11) { // Prevent Process_Terminate (Event Version(2))
        std::string imagefilename = parser.parse<std::string>(L"ImageFileName");
        //ProcessEntry pe({ (uint32_t)schema.event_opcode(), imagefilename });
        //fsm_handle::dispatch(pe);

        std::wcout << schema.task_name() << L"_" << schema.opcode_name();
        std::wcout << L" (" << schema.event_opcode() << L") ";
        std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");
        std::wcout << L" ProcessId=" << pid;
        auto ppid = GetParentProcessID(pid);
        if (ppid.has_value()) {
            std::cout << " ParentProcessId=" << ppid.value();
        }
        std::cout << " ImageFileName=" << imagefilename;
        
        std::wcout << std::endl;
    }
}

void file_rundown_callback(const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
    static int file_rundown_count = 0;

    if ((record.EventHeader.EventDescriptor.Opcode == 0) ||
        (record.EventHeader.EventDescriptor.Opcode == 32)) {  // FileRundown
        krabs::schema schema(record, trace_context.schema_locator);
        if (file_rundown_count++ >= 0) {
            std::wcout << schema.task_name() << L"_" << schema.opcode_name();
            std::wcout << L" (" << schema.event_opcode() << L") ";
            krabs::parser parser(schema);
            std::wstring filename = parser.parse<std::wstring>(L"FileName");
            std::wcout << L" FileName=" << filename;
            std::wcout << std::endl;
        }

        if (file_rundown_count == PRINT_LIMIT)
            std::wcout << schema.task_name() << L"_" << schema.opcode_name() << L"..." << std::endl;
    }
}

void hwconfig_callback(const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
    // only return some events for brevity
    if (record.EventHeader.EventDescriptor.Opcode == 10 || // CPU
        record.EventHeader.EventDescriptor.Opcode == 25 || // Platform
        record.EventHeader.EventDescriptor.Opcode == 33 || // DeviceFamily
        record.EventHeader.EventDescriptor.Opcode == 37) { // Boot Config Info
        krabs::schema schema(record, trace_context.schema_locator);
        std::wcout << L"task_name=" << schema.task_name();
        std::wcout << L" opcode=" << schema.event_opcode();
        std::wcout << L" opcode_name=" << schema.opcode_name();
        std::wcout << std::endl;
    }
}
/*
bool StartTempWatchdog(void)
{
    auto temp = LookForMCCTempPath();
    auto temp2 = LookForMCCMicrosoftStoreInstallPath();
    if (temp.has_value() && temp2.has_value()) {
        std::wofstream file(L"watchdog.log", std::ios::app | std::ios::out);
        auto reader1 = wil::make_folder_change_reader(temp.value().c_str(), true, wil::FolderChangeEvents::All, [&](wil::FolderChangeEvent event, PCWSTR fileName)
        {
                switch (event)
                {
                case wil::FolderChangeEvent::ChangesLost: {
                    std::wstringstream wss;
                    wss << L"Changes Lost, " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                case wil::FolderChangeEvent::Added: {
                    std::wstringstream wss;
                    wss << L"File Added, " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                case wil::FolderChangeEvent::Removed: {
                    std::wstringstream wss;
                    wss << L"File Removed, " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                case wil::FolderChangeEvent::Modified: {
                    std::wstringstream wss;
                    wss << L"File Modified, " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                case wil::FolderChangeEvent::RenameOldName: {
                    std::wstringstream wss;
                    wss << L"File Renamed (Old), " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                case wil::FolderChangeEvent::RenameNewName: {
                    std::wstringstream wss;
                    wss << L"File Renamed (New), " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                default: break;
                }
        });
        
        auto reader2 = wil::make_folder_change_reader((temp2.value()).c_str(), true, wil::FolderChangeEvents::All, [&](wil::FolderChangeEvent event, PCWSTR fileName)
            {
                switch (event)
                {
                case wil::FolderChangeEvent::ChangesLost: {
                    std::wstringstream wss;
                    wss << L"Changes Lost, " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                case wil::FolderChangeEvent::Added: {
                    std::wstringstream wss;
                    wss << L"File Added, " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                case wil::FolderChangeEvent::Removed: {
                    std::wstringstream wss;
                    wss << L"File Removed, " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                case wil::FolderChangeEvent::Modified: {
                    std::wstringstream wss;
                    wss << L"File Modified, " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                case wil::FolderChangeEvent::RenameOldName: {
                    std::wstringstream wss;
                    wss << L"File Renamed (Old), " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                case wil::FolderChangeEvent::RenameNewName: {
                    std::wstringstream wss;
                    wss << L"File Renamed (New), " << fileName << std::endl;
                    std::wcout << wss.str();
                    file << wss.str();
                    file.flush();
                    break;
                }
                default: break;
                }
            });
        while (true);
    }
    return false;
}*/
} // namespace mccinfo