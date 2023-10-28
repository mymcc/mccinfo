#pragma once

#include <string>
#include <iostream>
#include <utility>
#include <variant>
#include <type_traits>
#include <string_view>
#include <fsm/fsm.hpp>

namespace mccinfo {

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
};

template <typename... Ts> auto make_fsm_predicate_sequence(Ts &&... args) {
    return typename fsm_predicate_container<sizeof...(Ts)>::sequence{std::forward<Ts>(args)...};
}

krabs::predicates::opcode_is is_1 = krabs::predicates::opcode_is(1);
krabs::predicates::opcode_is is_2 = krabs::predicates::opcode_is(2);
auto is_launcher = krabs::predicates::property_is(L"ImageFileName", std::string("mcclauncher.exe"));
auto is_eac = krabs::predicates::property_is(L"ImageFileName", std::string("EasyAntiCheat.exe"));
krabs::predicates::any_of anyf({&is_1});

krabs::predicates::all_of allf({
    &is_launcher,
    &is_1,
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


struct pe_interface {
    virtual void handle_pe() = 0;
};

struct mcc_initial : public pe_interface {
    using transitions = ctfsm::type_map<std::pair<events::start, mcc_off>>;
    static constexpr std::string_view id{"INITIAL"};
    void on_enter()
    {
        std::cout << "INITIAL entered\n";
    }
    virtual void handle_pe() override {}
};

struct mcc_off : public pe_interface {
    using transitions = ctfsm::type_map<
        std::pair<events::reset, mcc_initial>,
        std::pair<events::launcher_start, mcc_loading>>;

    static constexpr std::string_view id{"MCC:OFF"};

    void on_enter()
    {
        std::cout << "mcc_off entered\n";
    }
    virtual void handle_pe() override {}
};

struct mcc_loading : public pe_interface {
    using transitions = ctfsm::type_map<
        std::pair<events::reset, mcc_initial>,
        std::pair<events::launcher_failed, mcc_off>,
        std::pair<events::launcher_end, mcc_on>>;

    static constexpr std::string_view id{"MCC:LOADING"};
    static constinit ctfsm::fsm<fsm::states::launch_initial> fsm;
    void on_enter()
    {
        std::cout << "mcc_loading entered\n";
    }
    virtual void handle_pe() override {}
};
struct mcc_on : public pe_interface {
    using transitions = ctfsm::type_map<
        std::pair<events::reset, mcc_initial>,
        std::pair<events::process_lost, mcc_off>>;

    static constexpr std::string_view id{"MCC:ON"};

    void on_enter()
    {
        std::cout << "mcc_on entered\n";
    }
    virtual void handle_pe() override {}
};
} // namespace states
} // namespace fsm

} // namespace mccinfo
