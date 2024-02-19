#pragma once

#include <ostream>
#include <memory>
#include "mccinfo/fsm/edges/edges.hpp"
#include "mccinfo/fsm/events/events.hpp"

namespace mccinfo {
namespace fsm {
namespace states {


template <typename _State>
//abstract_state
struct state {
    static auto clone_edges() {
        return _State::edges;
    }

    static events::event_t handle_trace_event(const EVENT_RECORD &record,
                                               const krabs::trace_context &trace_context) {
        using _EdgesType = decltype(_State::edges);
        
        if constexpr (std::is_same_v<_EdgesType, bool>){
            std::cout << "Edges are bool!\n";
        } else {
            std::cout << "Edges are of edge_type!!\n";

        }
        return {};
    }
};

struct state_context {
    template <typename _State>
    void handle_trace_event(std::wostringstream& woss, const EVENT_RECORD &record,
                                       const krabs::trace_context &trace_context) {
        std::optional<events::event_t> _evt;
        if (auto it = edge_context.find(utility::id<_State>); it != edge_context.end() ) {
            woss << L"\t\tState Context Cache: hit\n";
            _evt = it->second->handle_trace_event(woss, record, trace_context);

        } else {
            woss << "\t\tState Context Cache: miss\n";
            auto edges = _State::clone_edges();
            std::unique_ptr<edges::edge_container_base> ptr =
                std::make_unique<decltype(edges)>(edges);
            ptr->reset();
            _evt = ptr->handle_trace_event(woss, record, trace_context);
            edge_context.emplace(utility::id<_State>, std::move(ptr));
        }
        if (_evt.has_value())
            _event_queue.emplace(_evt.value(), utility::id<_State>);
    }

    std::optional<events::event_t> pop_event_from_queue() {
        if (_event_queue.size()) {
            const auto _evt_pair = _event_queue.front();
            _event_queue.pop();
            edge_context.erase(_evt_pair.second);

            return _evt_pair.first;
        } else {
            return std::nullopt;
        }
    }

    size_t event_queue_size() const {
        return _event_queue.size();
    }

  private:
    std::queue<std::pair<events::event_t, unsigned int>> _event_queue;
    std::unordered_map<unsigned int, std::unique_ptr<edges::edge_container_base>> edge_context;
};

template <typename StateMachine> class BonusStateVisitor {
  public:
  explicit BonusStateVisitor(const StateMachine &state_machine, const EVENT_RECORD& event_record, const krabs::trace_context& trace_context, state_context& sc, std::wostringstream& woss) : 
      state_machine_{state_machine}, 
      event_record_(event_record), 
      trace_context_(trace_context), 
      state_context_{sc}, 
      woss_{woss} {
}

  // Overload to handle states that have nested states
  template <typename CompositeState>
  void operator()(boost::sml::aux::string<boost::sml::sm<CompositeState>>) const
  {
    std::cout << boost::sml::aux::get_type_name<CompositeState>() << ':';
    state_machine_.template visit_current_states<boost::sml::aux::identity<CompositeState>>(*this);
  }

  // Overload to handle states without nested states and defines as: "state"_s
  template <char... Chars>
  void operator()(boost::sml::aux::string<boost::sml::aux::string<char, Chars...>> string_state) const
  {
    std::cout << string_state.c_str() << '\n';
  }

  // Overload to handle terminate states
  void operator()(boost::sml::aux::string<boost::sml::back::terminate_state> terminate_state) const
  {
    std::cout << terminate_state.c_str() << '\n';
  }

  // Overload to handle internal states
  void operator()(boost::sml::aux::string<boost::ext::sml::v1_1_9::front::internal>) const
  {
    // ignore internal states
  }

  // Overload to handle any other states, meaning states that are defined as: struct state{};)
  template <typename TSimpleState>
  void operator()(boost::sml::aux::string<TSimpleState>) const
  {
    auto ws = utility::ConvertBytesToWString(std::string(utility::make_type_name_minimal<TSimpleState>()));
    if (ws.has_value()) woss_ << L"\t\tCurrent State: " << ws.value() << L'\n';
    state_context_.handle_trace_event<TSimpleState>(woss_, event_record_, trace_context_);
  }

private:
  const StateMachine &state_machine_;
  const EVENT_RECORD &event_record_;
  const krabs::trace_context &trace_context_;
  state_context& state_context_;
  std::wostringstream& woss_;
};

template <typename StateMachine> class StatePrinter {
  public:
  explicit StatePrinter(const StateMachine &state_machine, std::wostringstream& woss) : 
      state_machine_{state_machine},
      woss_{woss} {
  }

  // Overload to handle states that have nested states
  template <typename CompositeState>
  void operator()(boost::sml::aux::string<boost::sml::sm<CompositeState>>) const
  {
    std::cout << boost::sml::aux::get_type_name<CompositeState>() << ':';
    state_machine_.template visit_current_states<boost::sml::aux::identity<CompositeState>>(*this);
  }

  // Overload to handle states without nested states and defines as: "state"_s
  template <char... Chars>
  void operator()(boost::sml::aux::string<boost::sml::aux::string<char, Chars...>> string_state) const
  {
    std::cout << string_state.c_str() << '\n';
  }

  // Overload to handle terminate states
  void operator()(boost::sml::aux::string<boost::sml::back::terminate_state> terminate_state) const
  {
    std::cout << terminate_state.c_str() << '\n';
  }

  // Overload to handle internal states
  void operator()(boost::sml::aux::string<boost::ext::sml::v1_1_9::front::internal>) const
  {
    // ignore internal states
  }

  // Overload to handle any other states, meaning states that are defined as: struct state{};)
  template <typename TSimpleState>
  void operator()(boost::sml::aux::string<TSimpleState>) const
  {
    auto ws = utility::ConvertBytesToWString(std::string(utility::make_type_name_minimal<TSimpleState>()));
    if (ws.has_value()) woss_ << L"\t\tResult State: " << ws.value() << L'\n';
  }

private:
  const StateMachine &state_machine_;
  std::wostringstream& woss_;
};

} //namespace states
} //namespace fsm
} //namespace mccinfo