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
        constexpr edge(predicates::details::sequence_base* seq, events::event_t evt)
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
        std::pair<predicates::details::sequence_base*, events::event_t> _edge;
};

template <typename... Tuples>
constexpr auto make_edges(Tuples&&... tuples) {
    return std::array<edge, sizeof...(Tuples)>{
        edge(std::get<0>(std::forward<Tuples>(tuples)), std::get<1>(std::forward<Tuples>(tuples)))...
    };
}

class edge_container_base {
    virtual void handle_trace_event(const EVENT_RECORD &record,
                               const krabs::trace_context &trace_context) = 0;
};

template <size_t N>
class edge_container : public edge_container_base
{
    template <typename... edges> 
	constexpr edge_container(edges... edgs) : _edges{edgs...} {
    static_assert(sizeof...(edgs) == N,
                    "The number of edges must match the template parameter N.");
	}

    virtual void handle_trace_event(const EVENT_RECORD &record,
                       const krabs::trace_context &trace_context) override final {
    }

  private:
    std::array<edge, N> _edges;
};



template <int _EdgeId> 
struct edge_t {
    constexpr edge_t(predicates::details::sequence_base *seq, events::event_t evt)
    {
        //edge_t<_EdgeId>::_seq = seq;
        //edge_t<_EdgeId>::_evt = evt;
    }
    static constexpr int id = _EdgeId;
    
};

// Factory function to create an edge_t with a unique ID

template <int n>
constexpr auto create_edge(predicates::details::sequence_base *seq, events::event_t evt) {

    return edge_t<n>{seq, evt};
}



template <typename _Edges> struct edge_map
{

};

template<typename T>
struct Proxy {
    T* ptr;

    template<typename U>
    constexpr auto operator ->*(U&& u) const {
        return std::pair<unsigned int, T*>{std::forward<U>(u), ptr};
    }
};


template<typename T>
constexpr Proxy<T> make_proxy(T* ptr) {
    return Proxy<T>{ptr};
}

struct trigger_handler {
    template <typename... triggers> 
    trigger_handler(triggers... trgrs) : _triggers{trgrs...} {

    }
    void handle(const EVENT_RECORD &record, const krabs::trace_context &trace_context) {
        for (auto &kv : _triggers) {
            kv.second->try_advance(record, trace_context);
        }
    }

  private:
    std::unordered_map<cometa::type_id_t, fsm::predicates::details::sequence_base*> _triggers;
};
}
}
}