#pragma once
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <initializer_list>
#include <any>
#include <utility>
#include "sequences.hpp"

namespace mccinfo {
namespace fsm {
namespace edges {

template <typename evt> static constexpr auto event_id = utility::type_hash<evt>::name_minimal;
template <typename evt> static constexpr auto event_ = std::pair<std::string_view, const events::event_base*>{
    event_id<evt>, &utility::type_hash<evt>::type};

template<typename T>
struct Proxy {
    T* ptr;

    template<typename U>
    constexpr auto operator ->*(U&& u) const {
        return std::pair<frozen::string, T*>{std::forward<U>(u), ptr};
    }
};

template <typename... T>
constexpr auto make_edges(T... pairs) {
    // Ensure that the pairs are expanded into the constructor correctly
    return frozen::unordered_map<frozen::string, predicates::details::sequence_base*, sizeof...(pairs)>{
        pairs...
    };
}

template <typename... T>
constexpr auto make_edge_events(T... pairs) {
    // Ensure that the pairs are expanded into the constructor correctly
    return frozen::unordered_map<frozen::string, const events::event_base*, sizeof...(pairs)>{
        pairs...
    };
}

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