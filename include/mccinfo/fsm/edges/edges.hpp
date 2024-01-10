#pragma once
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <initializer_list>
#include <any>
#include <utility>
#include <array>
#include <ostream>

#include "mccinfo/utility.hpp"
#include "mccinfo/fsm/events/events.hpp"
#include "sequences.hpp"

namespace mccinfo {
namespace fsm {
namespace edges {

enum class priority {
    weak,
};

class edge {
    public:
        constexpr edge(details::sequence_base* seq, events::event_t evt)
             : _edge(seq, evt) {}

        template <typename _Sm> 
        void traverse(_Sm *sm) const {
            std::visit([](auto &&evt) {
                auto new_evt = evt;
                sm->process_event(new_evt); 
            }, _edge.second);
        }
        
        priority get_priority() const {
            return _prio;
        }

        events::event_t get_event() const {
            return _edge.second;
        }

        virtual bool handle_trace_event(std::wostringstream& woss,
                                        const EVENT_RECORD &record,
                                        const krabs::trace_context &trace_context) {
            bool result = _edge.first->try_advance(record, trace_context);
            woss << L"Sequence Result: " << ((result) ? L"advanced" : L"nil") << L'\n';
            return _edge.first->is_complete();
        }

        void reset() {
            _edge.first->reset();
        }


    private:
        priority _prio{ priority::weak };
        std::pair<details::sequence_base*, events::event_t> _edge;
};



struct edge_container_base {
    virtual std::optional<events::event_t> handle_trace_event(std::wostringstream& woss, const EVENT_RECORD &record,
                               const krabs::trace_context &trace_context) = 0;
        virtual void reset() = 0;
};

template <size_t N>
struct edge_container : public edge_container_base {
    template <typename... _Edges> 
	constexpr edge_container(_Edges... edges) : _edges{edges...} {
    static_assert(sizeof...(edges) == N,
                    "The number of edges must match the template parameter N.");
	}

    virtual void reset() override
    {
        for (auto &_edge : _edges) {
            _edge.reset();
        }
    }

    virtual std::optional<events::event_t> handle_trace_event(std::wostringstream& woss, const EVENT_RECORD &record,
                       const krabs::trace_context &trace_context) override final {        
        std::vector<std::pair<edges::priority, events::event_t>> hot_events;

        for (auto &_edge : _edges) {
            if (_edge.handle_trace_event(woss, record, trace_context)) {
                hot_events.push_back({_edge.get_priority(), _edge.get_event()});
            }
        }
        if (hot_events.size()) {
            events::event_t new_event = hot_events[0].second;
            return new_event;
        }
        else
            return std::nullopt;
    }

  private:
    std::array<edge, N> _edges;
};

template <typename... Tuples>
constexpr auto make_edges(Tuples&&... tuples) {
    return edge_container<sizeof...(Tuples)>{
        edge(std::get<0>(std::forward<Tuples>(tuples)), std::get<1>(std::forward<Tuples>(tuples)))...
    };
}
}
}
}