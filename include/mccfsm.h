#pragma once

#include <string>
#include <iostream>
#include <utility>
#include <variant>
#include <type_traits>
#include <tinyfsm.hpp>

namespace mccinfo {
	
	struct fsm_predicate_base {
		virtual ~fsm_predicate_base() {}
		virtual std::shared_ptr<krabs::predicates::details::predicate_base> GetBase() const = 0;
	};

	template <typename T>
	struct fsm_predicate_derived : public fsm_predicate_base {
		static_assert(std::is_base_of<krabs::predicates::details::predicate_base, T>::value,
			"T must be derived from details::predicate_base");

		std::shared_ptr<T> ptr;

		fsm_predicate_derived(T predicate) : ptr(std::make_shared<T>(std::move(predicate))) {}

		virtual std::shared_ptr<krabs::predicates::details::predicate_base> GetBase() const override {
			return ptr;
		}

		T& Get() {
			return *ptr;
		}
	};

	//struct fsm_predicate {
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
		fsm_predicate(T predicate) : ptr(std::make_shared<fsm_predicate_derived<T>>(std::move(predicate))) {
		}

		template <typename T>
		T& Get() {
			auto derived = dynamic_cast<fsm_predicate_derived<T>*>(ptr.get());
			if (derived) {
				return derived->Get();
			}
			else {
				throw std::bad_cast();
			}
		}
	};

	template <std::size_t N>
	struct fsm_predicate_container {
		using sequence = std::array<fsm_predicate, N>;
	};

	template <typename... Ts> auto make_fsm_predicate_sequence(Ts &&...args) {
		return typename fsm_predicate_container<sizeof...(Ts)>::sequence {
			std::forward<Ts>(args)...
		};
	}

	krabs::predicates::opcode_is is_1 = krabs::predicates::opcode_is(1);
	krabs::predicates::opcode_is is_2 = krabs::predicates::opcode_is(2);
	auto is_launcher = krabs::predicates::property_is(L"ImageFileName", std::string("mcclauncher.exe"));
	auto is_eac = krabs::predicates::property_is(L"ImageFileName", std::string("EasyAntiCheat.exe"));
	krabs::predicates::any_of anyf ({
		&is_1
	});

	auto myseq = make_fsm_predicate_sequence(is_1, anyf);

	typedef std::pair<uint32_t, std::string> process_entry;

	template <typename... Ts> constexpr auto make_process_entry_sequence(Ts &&...args) {
		return std::array<process_entry, sizeof...(Ts)>{
			process_entry(std::forward<Ts>(args))...};
	}

	template <typename T>
	class fsm_etw_provider : public T {
	public:
		void swap_filter(const krabs::event_filter&& f) {
			try {
				//this->filters_.clear();
				//std::wcout << L"clear() good" << std::endl;
				this->filters_.emplace_back(f);
				std::wcout << L"emplace_back() good" << std::endl;
			}
			catch (const std::exception& e) {
				std::cerr << "Caught exception: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "Caught unknown exception." << std::endl;
			}
		}
		void set_on_event_end_callback(std::function<void()>&& cb) {
			this->event_end_callback_ = cb;
		}
	};

	using fsm_kernel_process_provider = fsm_etw_provider<krabs::kernel::process_provider>;
	using fsm_kernel_file_io_provider = fsm_etw_provider<krabs::kernel::file_io_provider>;

	struct Off;

	struct MyEvent {
		virtual ~MyEvent() = default;
	};

	template <typename T>
	struct SequenceStart : public MyEvent {
		SequenceStart(T* _provider) : provider(_provider) {}
		T* provider;
	};

	template <typename T>
	struct SequenceAdvance : public MyEvent {
		SequenceAdvance(T* _provider) : provider(_provider) {}
		T* provider;
	};

	template <typename T>
	struct SequenceEnd : public MyEvent {
		SequenceEnd(T* _provider) : provider(_provider) {}
		T* provider;
	};

	struct mccfsm : public tinyfsm::Fsm<mccfsm> {
		virtual void react(MyEvent const&) {};
		virtual void entry(void) { };
		virtual void exit(void) { };
	};

	struct On : mccfsm {
		void entry() override { std::cout << "MCC has been started!" << std::endl; }
		void react(MyEvent const& evt) override {
			if (typeid(evt) == typeid(SequenceAdvance<fsm_kernel_process_provider>)) {
				//auto& typedEvent = static_cast<const SequenceAdvance<fsm_kernel_process_provider>&>(evt);
				// Do something special for fsm_kernel_process_provider
			}
			else {
				// Handle generic case
			}
		};

	};

	struct Off : mccfsm {
		void entry() override { std::cout << "MCC has been closed!" << std::endl; }
		void react(MyEvent const& evt) override {
			std::wcout << "react(evt)" << std::endl;
			if (auto typedEvent = dynamic_cast<const SequenceAdvance<fsm_kernel_process_provider>*>(&evt)) {
				std::wcout << L" SequenceAdvance " << std::endl;
				seq += 1;

				if (seq == to_on_sequence.size()) {
					transit<On>();
				}

				else {
					
					std::wcout << L"filter good" << std::endl;
					auto provider = typedEvent->provider;
					std::wcout << L"provider* copy good" << std::endl;
					auto loc_process_rundown_callback = [provider](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
						krabs::schema schema(record, trace_context.schema_locator);
						std::wcout << L"schema() good" << std::endl;
						krabs::parser parser(schema);
						std::wcout << L"parser() good" << std::endl;
						if (schema.event_opcode() != 11) { // Prevent Process_Terminate (Event Version(2))
							std::string imagefilename = parser.parse<std::string>(L"ImageFileName");
							std::wcout << schema.task_name() << L"_" << schema.opcode_name();
							std::wcout << L" (" << schema.event_opcode() << L") ";
							std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");
							std::wcout << L" ProcessId=" << pid;
							//auto ppid = GetParentProcessID(pid);
							//if (ppid.has_value()) {
							//	std::cout << " ParentProcessId=" << ppid.value();
							//}
							std::cout << " ImageFileName=" << imagefilename;
							std::wcout << std::endl;
							mccfsm::dispatch(SequenceAdvance(provider));
						}
					};
					krabs::event_filter filter { to_on_sequence[seq].Get<krabs::predicates::all_of>() };
					filter.add_on_event_callback(std::move(loc_process_rundown_callback));
					std::wcout << L"filter_add_on_event() good" << std::endl;
					//provider->add_filter(filter);
					//provider->swap_filter(std::move(filter));
				}
				// Do something special for fsm_kernel_process_provider
			}
			else if (auto typedEvent = dynamic_cast<const SequenceStart<fsm_kernel_process_provider>*>(&evt)) {
				std::wcout << L" SequenceStart" << std::endl;
				krabs::event_filter filter { to_on_sequence[seq].Get<krabs::predicates::all_of>() };
				auto provider = typedEvent->provider;
				auto loc_process_rundown_callback = [provider](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
					krabs::schema schema(record, trace_context.schema_locator);
					krabs::parser parser(schema);
					if (schema.event_opcode() != 11) { // Prevent Process_Terminate (Event Version(2))
						std::string imagefilename = parser.parse<std::string>(L"ImageFileName");
						std::wcout << schema.task_name() << L"_" << schema.opcode_name();
						std::wcout << L" (" << schema.event_opcode() << L") ";
						std::uint32_t pid = parser.parse<std::uint32_t>(L"ProcessId");
						std::wcout << L" ProcessId=" << pid;
						//auto ppid = GetParentProcessID(pid);
						//if (ppid.has_value()) {
						//	std::cout << " ParentProcessId=" << ppid.value();
						//}
						std::cout << " ImageFileName=" << imagefilename;
						std::wcout << std::endl;
						mccfsm::dispatch(SequenceAdvance(provider));
					}
				};

				filter.add_on_event_callback(loc_process_rundown_callback);
				provider->swap_filter(std::move(filter));
			}
			else {
				// Handle generic case
				std::wcout << L"Unable to react to event " << typeid(evt).name() << std::endl;
			}
		}
	private:
		size_t seq = 0;
		fsm_predicate_container<2>::sequence to_on_sequence{ make_fsm_predicate_sequence(
			krabs::predicates::all_of({ &is_1 , &is_launcher}),
			krabs::predicates::all_of({ &is_2 , &is_launcher})
		)};
	};
}

FSM_INITIAL_STATE(mccinfo::mccfsm, mccinfo::Off)
using fsm_handle = mccinfo::mccfsm;