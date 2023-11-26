#pragma once

#include <memory>
#include "..\edges\edges.hpp"
#include "..\events\events.hpp"

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
    //events::event_t handle_trace_event(const EVENT_RECORD &record,
    void handle_trace_event(const EVENT_RECORD &record,
                                       const krabs::trace_context &trace_context) {
        if (auto it = edge_context.find(utility::id<_State>); it != edge_context.end() ) {
            //return findit->first;
            std::cout << "state context cache hit\n";
            it->second->handle_trace_event(record, trace_context);

        } else {
            std::cout << "state context cache miss\n";
            auto edges = _State::clone_edges();
            std::unique_ptr<edges::edge_container_base> ptr =
                std::make_unique<decltype(edges)>(std::move(edges));
            ptr->handle_trace_event(record, trace_context);
            edge_context.emplace(utility::id<_State>, std::move(ptr));
        }
    }

  private:
    std::unordered_map<unsigned int, std::unique_ptr<edges::edge_container_base>> edge_context;
};

template <typename StateMachine> class BonusStateVisitor {
  public:
  explicit BonusStateVisitor(const StateMachine &state_machine, const EVENT_RECORD& event_record, const krabs::trace_context& trace_context, state_context& sc) : 
      state_machine_{state_machine}, 
      event_record_(event_record), 
      trace_context_(trace_context), 
      state_context_{sc} {
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
    std::cout << boost::sml::aux::get_type_name<TSimpleState>() << '\n';
    std::cout << utility::make_type_name_minimal<TSimpleState>() << '\n';
    //auto self = TSimpleState();
    TSimpleState::handle_trace_event(event_record_, trace_context_);
    state_context_.handle_trace_event<TSimpleState>(event_record_, trace_context_);
  }

private:
  const StateMachine &state_machine_;
  const EVENT_RECORD &event_record_;
  const krabs::trace_context &trace_context_;
  state_context& state_context_;
};
} //namespace states
} //namespace fsm
} //namespace mccinfo