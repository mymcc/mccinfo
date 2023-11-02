#pragma once

#include <atomic>
#include <string>
#include <iostream>
#include <utility>
#include <variant>
#include <type_traits>
#include <string_view>
#include <type_traits>
#include <concepts>
#include <fsm/fsm.hpp>
#include <lockfree/lockfree.hpp>

namespace mccinfo {
    class atomic_mutex {
    public:
        void lock() {
            while (flag.exchange(true, std::memory_order_relaxed));
            std::atomic_thread_fence(std::memory_order_acquire);
        }

        void unlock() {
            std::atomic_thread_fence(std::memory_order_release);
            flag.store(false, std::memory_order_relaxed);
        }

    private:
        std::atomic<bool> flag{ false };
    };

    class atomic_guard {
    public:
        atomic_guard(atomic_mutex& mutex) : m_Mutex(mutex) { m_Mutex.lock(); }
        ~atomic_guard() { m_Mutex.unlock(); }

    private:
        atomic_mutex& m_Mutex;
    };
struct fsm_predicate_base {
    virtual ~fsm_predicate_base() {
    }
    virtual std::shared_ptr<krabs::predicates::details::predicate_base> GetBase() const = 0;
};

template <typename T> struct fsm_predicate_derived : public fsm_predicate_base {
    static_assert(std::is_base_of<krabs::predicates::details::predicate_base, T>::value,
                  "T must be derived from details::predicate_base");

    std::shared_ptr<T> ptr;

    fsm_predicate_derived(T predicate) : ptr(std::make_shared<T>(std::move(predicate))) {
    }

    virtual std::shared_ptr<krabs::predicates::details::predicate_base> GetBase() const override {
        return ptr;
    }

    T &Get() {
        return *ptr;
    }
};

// struct fsm_predicate {
//	std::shared_ptr<krabs::predicates::details::predicate_base> ptr;
//	template<typename T>
//	fsm_predicate(T predicate) : ptr(std::make_shared<T>(std::move(predicate))) {
//		static_assert(std::is_base_of<krabs::predicates::details::predicate_base, T>::value,
//			"T must be derived from details::predicate_base");
//	}
//};

struct fsm_predicate {
    std::shared_ptr<fsm_predicate_base> ptr;

    template <typename T>
    fsm_predicate(T predicate)
        : ptr(std::make_shared<fsm_predicate_derived<T>>(std::move(predicate))) {
    }

    template <typename T> T &Get() {
        auto derived = dynamic_cast<fsm_predicate_derived<T> *>(ptr.get());
        if (derived) {
            return derived->Get();
        } else {
            throw std::bad_cast();
        }
    }
};

template <std::size_t N> struct fsm_predicate_container {
    using sequence = std::array<fsm_predicate, N>;
    using sequence_set = std::array<sequence, N>;
};

template <typename... Ts> auto make_fsm_predicate_sequence(Ts &&... args) {
    return typename fsm_predicate_container<sizeof...(Ts)>::sequence{std::forward<Ts>(args)...};
}

krabs::predicates::opcode_is is_1 = krabs::predicates::opcode_is(1);
krabs::predicates::opcode_is is_2 = krabs::predicates::opcode_is(2);
auto is_launcher = krabs::predicates::property_is(L"ImageFileName", std::string("mcclauncher.exe"));
auto is_eac = krabs::predicates::property_is(L"ImageFileName", std::string("EasyAntiCheat.exe"));
auto is_mcc = krabs::predicates::property_is(L"ImageFileName", std::string("MCC-Win64-Shipping.exe"));
krabs::predicates::any_of anyf({&is_1});

krabs::predicates::all_of allf({
    &is_launcher,
    &is_1,
});

krabs::predicates::all_of launcher_start_pred({
    &is_launcher,
    &is_1,
});

krabs::predicates::all_of launcher_end_pred({
    &is_launcher,
    &is_2,
});

krabs::predicates::all_of eac_start_pred({
    &is_eac,
    &is_1,
});

krabs::predicates::all_of mcc_process_lossed({
    &is_mcc,
    &is_2,
    });

auto myseq = make_fsm_predicate_sequence(is_1, anyf);

typedef std::pair<uint32_t, std::string> process_entry;

template <typename... Ts> constexpr auto make_process_entry_sequence(Ts &&... args) {
    return std::array<process_entry, sizeof...(Ts)>{process_entry(std::forward<Ts>(args))...};
}

template <typename T> class fsm_etw_provider : public T {
  public:
    void swap_filter(const krabs::event_filter &&f) {
        try {
            // this->filters_.clear();
            // std::wcout << L"clear() good" << std::endl;
            this->filters_.emplace_back(f);
            std::wcout << L"emplace_back() good" << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "Caught exception: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Caught unknown exception." << std::endl;
        }
    }
    void set_on_event_end_callback(std::function<void()> &&cb) {
        this->event_end_callback_ = cb;
    }
};

using fsm_kernel_process_provider = fsm_etw_provider<krabs::kernel::process_provider>;
using fsm_kernel_file_io_provider = fsm_etw_provider<krabs::kernel::file_io_provider>;
constexpr size_t MAX_TASK_NAME_LENGTH = 256;
constexpr size_t MAX_IMAGE_FILENAME_LENGTH = 256;

struct TraceEvent
{
    wchar_t task_name[MAX_TASK_NAME_LENGTH];
    wchar_t opcode_name[MAX_TASK_NAME_LENGTH];
    int event_opcode;
    std::uint32_t pid;
    char imagefilename[MAX_IMAGE_FILENAME_LENGTH];
};

void construct_trace_event(TraceEvent& pe, const krabs::schema& schema, krabs::parser& parser) {
    std::string imagefilename = parser.parse<std::string>(L"ImageFileName");
    std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");

    wcsncpy(pe.task_name, schema.task_name(), MAX_TASK_NAME_LENGTH - 1);
    pe.task_name[MAX_TASK_NAME_LENGTH - 1] = L'\0'; // Ensure null-termination

    wcsncpy(pe.opcode_name, schema.opcode_name(), MAX_TASK_NAME_LENGTH - 1);
    pe.opcode_name[MAX_TASK_NAME_LENGTH - 1] = L'\0'; // Ensure null-termination

    pe.event_opcode = schema.event_opcode();
    pe.pid = pid;

    // Assuming imagefilename is a std::string or similar
    strncpy(pe.imagefilename, imagefilename.c_str(), MAX_IMAGE_FILENAME_LENGTH - 1);
    pe.imagefilename[MAX_IMAGE_FILENAME_LENGTH - 1] = '\0';
}


template <typename T, std::size_t N>
class predicate_sequence {
public:
    template <typename... Predicates>
    constexpr predicate_sequence(Predicates... preds) : _seq{ preds... }, current_index(0) {
        static_assert(sizeof...(preds) == N, "The number of predicates must match the template parameter N.");
    }
    constexpr predicate_sequence() : _seq{}, current_index(0) {
        static_assert(N == 0, "This constructor can only be used when N is 0.");
    }
    bool is_complete() const { return current_index == N; }
    bool try_advance(const EVENT_RECORD& record, const krabs::trace_context& trace_context) const {
        if (!is_complete()) {
            if ((*_seq[current_index])(record, trace_context)) {
                current_index += 1;
                return true;
            }
        }
        return false;
    }
    T consume_next_event() const {
        if (is_complete()) {
            current_index = 0;
            return T();
        }
        else {
            throw std::runtime_error("big no no");
        }
    }
private:
    std::array<krabs::predicates::details::predicate_base*, N> _seq;
    mutable size_t current_index = 0;
};
template <typename T, typename... Predicates>
predicate_sequence(T, Predicates...) -> predicate_sequence<T, sizeof...(Predicates)>;

template <typename T, typename... Predicates>
constexpr auto make_predicate_sequence(Predicates... preds) {
    return predicate_sequence<T, sizeof...(preds)>{preds...};
}

static lockfree::mpmc::Queue<TraceEvent, 10000> s_EventQueue;


namespace fsm {
namespace events {
struct start {};
struct launcher_start{};
struct launcher_failed{};
struct launcher_end{};
struct process_found{};
struct process_lost{};
struct reset {};

struct launch_start {};
struct eac{};
struct no_eac{};
struct launch_reset {};

struct process_event{};
} // namespace events

namespace states {
    struct mcc_initial;
    struct mcc_off;
    struct mcc_loading;
    struct mcc_on;

    struct launch_initial;
    struct launch_start;
    struct launch_with_eac;
    struct launch_no_eac;

    struct launch_initial {
        using transitions = ctfsm::type_map<std::pair<events::launch_start, launch_start>>;
        static constexpr std::string_view id{"LAUNCH INITIAL"};
    };

    struct launch_start {
        using transitions = ctfsm::type_map<
            std::pair<events::launch_reset, launch_initial>,
            std::pair<events::eac, launch_with_eac>,
            std::pair<events::no_eac, launch_no_eac>>;

        static constexpr std::string_view id{"LAUNCH START"};
    };

    struct launch_with_eac {
        using transitions = ctfsm::type_map<
            std::pair<events::launch_reset, launch_initial>>;

        static constexpr std::string_view id{"LAUNCH WITH EAC"};
    };

    struct launch_no_eac {
        using transitions = ctfsm::type_map<
            std::pair<events::launch_reset, launch_initial>>;

        static constexpr std::string_view id{"LAUNCH WITH NO EAC"};
    };

static atomic_mutex s_Lock;

template <typename Derived>
struct trace_event_handler {
    template <typename fsm_type>
    void handle_trace_event(fsm_type* fsm, const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
        atomic_guard lk(s_Lock);

        if (Derived::seq.try_advance(record, trace_context)) {
            std::cout << "handling trace event" << std::endl;
            krabs::schema schema(record, trace_context.schema_locator);
            krabs::parser parser(schema);

            if (schema.event_opcode() != 11) { // Prevent Process_Terminate (Event Version(2))
                TraceEvent pe;
                construct_trace_event(pe, schema, parser);
                s_EventQueue.Push(pe);
            }
        }
        if (Derived::seq.is_complete()) {
            std::cout << "should transition" << std::endl;
            fsm->handle_event(Derived::seq.consume_next_event());
        }
    }
};

struct mcc_initial : public trace_event_handler<mcc_initial> {
    using transitions = ctfsm::type_map<std::pair<events::start, mcc_off>>;
    static constexpr std::string_view id{"INITIAL"};
    static constexpr predicate_sequence<events::start, 0> seq{}; //meant to be unused

    void on_enter()
    {
        std::cout << "INITIAL entered" << std::endl;
    }
};

struct mcc_off : public trace_event_handler<mcc_off> {
    using transitions = ctfsm::type_map<
        std::pair<events::reset, mcc_initial>,
        std::pair<events::launcher_start, mcc_loading>>;

    static constexpr std::string_view id{"MCC:OFF"};

    static constexpr predicate_sequence<events::launcher_start, 2> seq{ &launcher_start_pred, &eac_start_pred};

    void on_enter()
    {
        std::cout << "mcc_off entered" << std::endl;
        std::cout << "checking for mcc_process" << std::endl;
        if (utils::LookForMCCProcessID().has_value()) {
            std::cout << "mcc process found, should transition to mcc_on" << std::endl;

        }
    }
};

struct mcc_loading : public trace_event_handler<mcc_loading> {
    using transitions = ctfsm::type_map<
        std::pair<events::reset, mcc_initial>,
        std::pair<events::launcher_failed, mcc_off>,
        std::pair<events::launcher_end, mcc_on>>;

    static constexpr std::string_view id{"MCC:LOADING"};
    //static constinit ctfsm::fsm<fsm::states::launch_initial> fsm;

    static constexpr auto seq = make_predicate_sequence<events::launcher_end>(&launcher_end_pred);

    void on_enter()
    {
        std::cout << "mcc_loading entered" << std::endl;
    }
};
struct mcc_on : public trace_event_handler<mcc_on> {
    using transitions = ctfsm::type_map<
        std::pair<events::reset, mcc_initial>,
        std::pair<events::process_lost, mcc_off>>;

    static constexpr std::string_view id{"MCC:ON"};
    static constexpr predicate_sequence<events::process_lost, 1> seq{ &mcc_process_lossed };

    void on_enter()
    {
        std::cout << "mcc_on entered" << std::endl;
    }
};
} // namespace states
} // namespace fsm
} // namespace mccinfo
