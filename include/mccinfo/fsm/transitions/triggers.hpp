#pragma once

#include <cstdint>
#include <cometa.hpp>
#include <cident.h>
#include "sequences.hpp"

namespace mccinfo {
namespace fsm {
namespace transitions {

template <typename evt, typename seq>
constexpr std::pair<cometa::type_id_t, predicates::details::sequence_base*> make_trigger(seq* _seq) {
    return {cometa::ctypeid<evt>(), _seq};
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