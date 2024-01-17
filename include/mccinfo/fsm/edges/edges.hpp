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
             : edge_(seq, evt) {}

        template <typename _Sm> 
        void traverse(_Sm *sm) const {
            std::visit([](auto &&evt) {
                auto new_evt = evt;
                sm->process_event(new_evt); 
            }, edge_.second);
        }
        
        priority get_priority() const {
            return prio_;
        }

        events::event_t get_event() const {
            return edge_.second;
        }

        virtual bool handle_trace_event(std::wostringstream& woss,
                                        const EVENT_RECORD &record,
                                        const krabs::trace_context &trace_context) {
            bool result = edge_.first->try_advance(record, trace_context);
            woss << L"Sequence Result: " << ((result) ? L"advanced" : L"nil") << L'\n';
            return edge_.first->is_complete();
        }

        void reset() {
            edge_.first->reset();
        }


    private:
        priority prio_{ priority::weak };
        std::pair<details::sequence_base*, events::event_t> edge_;
};



struct edge_container_base {
    virtual std::optional<events::event_t> handle_trace_event(std::wostringstream& woss, const EVENT_RECORD &record,
                               const krabs::trace_context &trace_context) = 0;
        virtual void reset() = 0;
};

template <size_t N>
struct edge_container : public edge_container_base {
    template <typename... _Edges> 
	constexpr edge_container(_Edges... edges) : edges_{edges...} {
    static_assert(sizeof...(edges) == N,
                    "The number of edges must match the template parameter N.");
	}

    virtual void reset() override
    {
        for (auto &edge_ : edges_) {
            edge_.reset();
        }
    }

    virtual std::optional<events::event_t> handle_trace_event(std::wostringstream& woss, const EVENT_RECORD &record,
                       const krabs::trace_context &trace_context) override final {        
        std::vector<std::pair<edges::priority, events::event_t>> hot_events;

        for (auto &edge_ : edges_) {
            if (edge_.handle_trace_event(woss, record, trace_context)) {
                hot_events.push_back({edge_.get_priority(), edge_.get_event()});
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
    std::array<edge, N> edges_;
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