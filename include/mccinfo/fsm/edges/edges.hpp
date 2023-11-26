#pragma once
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <initializer_list>
#include <any>
#include <utility>
#include <array>

#include "..\..\utility.hpp"
#include "..\events\events.hpp"
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

    private:
        priority _prio{priority::weak};
        std::pair<details::sequence_base*, events::event_t> _edge;
};



struct edge_container_base {
    virtual void handle_trace_event(const EVENT_RECORD &record,
                               const krabs::trace_context &trace_context) = 0;
};

template <size_t N>
struct edge_container : public edge_container_base {
    template <typename... _Edges> 
	constexpr edge_container(_Edges... edges) : _edges{edges...} {
    static_assert(sizeof...(edges) == N,
                    "The number of edges must match the template parameter N.");
	}

    virtual void handle_trace_event(const EVENT_RECORD &record,
                       const krabs::trace_context &trace_context) override final {
    std::cout << "handling an event from inside an edge container!\n";
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