#include "mccinfo.h"
#include "utils/utils.hpp"
#include "mccfsm.h"

namespace mccinfo {

constexpr std::array<size_t, 4> name_events{0, 32, 35, 36};

constexpr std::array<size_t, 5> events{69, 70, 71, 74, 75};

const std::unordered_map<size_t, std::wstring_view> eventToWstring{
    {69, L"SetInfo"}, {70, L"Delete"}, {71, L"Rename"}, {74, L"QueryInfo"}, {75, L"FSControl"}};

std::unordered_map<uint32_t *, std::wstring> FileKeyMap;

constexpr std::array<uint32_t, 5> process_events{1, 2, 3, 4, 11};

krabs::event_filter MakeProcessFilter(void) {
    static std::array<krabs::predicates::opcode_is, 5> opcode_predicates{
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
    static krabs::event_filter filter{krabs::predicates::and_filter(krabs::predicates::any_of({
                                                                        &opcode_predicates[0],
                                                                        &opcode_predicates[1],
                                                                        &opcode_predicates[2],
                                                                        &opcode_predicates[3],
                                                                    }),
                                                                    krabs::predicates::all_of({
                                                                        &version_is,
                                                                    }))};
    return filter;
}
krabs::event_filter MakeProcessFilter2(void) {
    auto start = krabs::predicates::opcode_is(1);
    auto name =
        krabs::predicates::property_equals(L"ImageFileName", std::wstring(L"mcclauncher.exe"));

    krabs::event_filter filter{krabs::predicates::all_of({&start, &name})};

    return filter;
}
#define PRINT_LIMIT 3 // only print a few events for brevity
void process_rundown_callback(const EVENT_RECORD &record,
                              const krabs::trace_context &trace_context);
void process_rundown_callback2(const EVENT_RECORD &record,
                               const krabs::trace_context &trace_context);
void file_rundown_callback(const EVENT_RECORD &record, const krabs::trace_context &trace_context);
void hwconfig_callback(const EVENT_RECORD &record, const krabs::trace_context &trace_context);

void PrintEvent(TraceEvent& evt);
void FlushEventQueue() {
    TraceEvent pe;
    while (mccinfo::s_EventQueue.Pop(pe)) {
        PrintEvent(pe);
    }
}

void PrintEvent(TraceEvent& evt) {
    if (evt.event_opcode != 11) { // Prevent Process_Terminate (Event Version(2))
        std::string imagefilename = evt.imagefilename;
        // ProcessEntry pe({ (uint32_t)schema.event_opcode(), imagefilename });
        // fsm_handle::dispatch(pe);

        std::wcout << evt.task_name << L"_" << evt.opcode_name;
        std::wcout << L" (" << evt.event_opcode << L") ";
        std::uint32_t pid = evt.pid;
        std::wcout << L" ProcessId=" << pid;
        auto ppid = utils::details::GetParentProcessID(pid);
        if (ppid.has_value()) {
            std::cout << " ParentProcessId=" << ppid.value();
        }
        std::cout << " ImageFileName=" << imagefilename;

        std::wcout << std::endl;
    }
}

void ProcessPE(TraceEvent& evt) {
    if (evt.event_opcode != 11) { // Prevent Process_Terminate (Event Version(2))
        std::string imagefilename = evt.imagefilename;
        // ProcessEntry pe({ (uint32_t)schema.event_opcode(), imagefilename });
        // fsm_handle::dispatch(pe);

        std::wcout << evt.task_name << L"_" << evt.opcode_name;
        std::wcout << L" (" << evt.event_opcode << L") ";
        std::uint32_t pid = evt.pid;
        std::wcout << L" ProcessId=" << pid;
        auto ppid = utils::details::GetParentProcessID(pid);
        if (ppid.has_value()) {
            std::cout << " ParentProcessId=" << ppid.value();
        }
        std::cout << " ImageFileName=" << imagefilename;

        std::wcout << std::endl;
    }
}
static ctfsm::fsm<fsm::states::mcc_initial> s_fsm;
bool StartETW(void) {

    // we will make different filters, attach different callbacks that dispatch their respective
    // event. like a successful predicate filter should elicit next_predicate_in_seq but a process
    // end for mcc should elicit terminate, but only if its a pid we recognized to be on and were
    // tracking

    
    s_fsm.handle_event<fsm::events::start>();

    krabs::kernel_trace trace(L"kernel_trace");

    // fsm_kernel_process_provider process_provider;
    // krabs::kernel::process_provider process_provider2;
    // fsm_handle::dispatch(SequenceStart(&process_provider));
    krabs::kernel::process_provider process_provider;
    krabs::event_filter filter { krabs::predicates::any_of({ &is_launcher, &is_eac, &is_mcc }) };
    filter.add_on_event_callback(process_rundown_callback2);
    process_provider.add_filter(filter);

    trace.enable(process_provider);

    // process_provider.add_filter

    // krabs::kernel::disk_file_io_provider file_io_provider;
    // file_io_provider.add_on_event_callback(file_rundown_callback);
    ////trace.enable(file_io_provider);
    //
    // krabs::kernel_provider hwconfig_provider(0, krabs::guids::event_trace_config);
    // hwconfig_provider.add_on_event_callback(hwconfig_callback);
    // trace.enable(hwconfig_provider);

    // krabs::kernel_trace trace2(L"kernel_trace2");
    // krabs::kernel::disk_file_io_provider

    std::cout << " - starting trace" << std::endl;

    std::thread thread([&trace]() { trace.start(); });

    // We will wait for all start events to be processed.
    // By default ETW buffers are flush when full, or every second otherwise
    //Sleep(90000);


    //Sleep(10000);
    //trace.stop();
    
    while (true) {
        std::string input;
        std::getline(std::cin, input);
        if (input == "q") {
            std::cout << std::endl << " - stopping trace" << std::endl;
            trace.stop();
            FlushEventQueue();
            break;
        }
    }
    thread.join();
    return true;
}

void process_rundown_callback2(const EVENT_RECORD &record,
                               const krabs::trace_context &trace_context) {
    s_fsm.invoke_on_current([&](auto&& current, auto& fsm) {
        current.handle_trace_event<decltype(s_fsm)>(&s_fsm, record, trace_context);
    });
} 
void process_rundown_callback(const EVENT_RECORD &record,
                              const krabs::trace_context &trace_context) {
    krabs::schema schema(record, trace_context.schema_locator);
    krabs::parser parser(schema);

    if (schema.event_opcode() != 11) { // Prevent Process_Terminate (Event Version(2))
        std::string imagefilename = parser.parse<std::string>(L"ImageFileName");
        // ProcessEntry pe({ (uint32_t)schema.event_opcode(), imagefilename });
        // fsm_handle::dispatch(pe);

        std::wcout << schema.task_name() << L"_" << schema.opcode_name();
        std::wcout << L" (" << schema.event_opcode() << L") ";
        std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");
        std::wcout << L" ProcessId=" << pid;
        auto ppid = utils::details::GetParentProcessID(pid);
        if (ppid.has_value()) {
            std::cout << " ParentProcessId=" << ppid.value();
        }
        std::cout << " ImageFileName=" << imagefilename;

        std::wcout << std::endl;
    }
}

void file_rundown_callback(const EVENT_RECORD &record, const krabs::trace_context &trace_context) {
    static int file_rundown_count = 0;

    if ((record.EventHeader.EventDescriptor.Opcode == 0) ||
        (record.EventHeader.EventDescriptor.Opcode == 32)) { // FileRundown
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

void hwconfig_callback(const EVENT_RECORD &record, const krabs::trace_context &trace_context) {
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
        auto reader1 = wil::make_folder_change_reader(temp.value().c_str(), true,
wil::FolderChangeEvents::All, [&](wil::FolderChangeEvent event, PCWSTR fileName)
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

        auto reader2 = wil::make_folder_change_reader((temp2.value()).c_str(), true,
wil::FolderChangeEvents::All, [&](wil::FolderChangeEvent event, PCWSTR fileName)
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