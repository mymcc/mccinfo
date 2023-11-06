#pragma once

#include <utility>
#include <typeinfo>
#include <typeindex>
#include "sequences.hpp"

namespace mccinfo {
namespace fsm {
namespace transitions {

template <typename evt, typename seq>
std::pair<std::type_index, predicates::details::sequence_base*> make_trigger(seq* _seq) {
    return std::make_pair<std::type_index, predicates::details::sequence_base*>(typeid(evt), _seq);
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
    std::unordered_map<std::type_index, fsm::predicates::details::sequence_base*> _triggers;
};
}
}
}