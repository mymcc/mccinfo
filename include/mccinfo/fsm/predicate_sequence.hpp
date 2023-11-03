#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <krabs/krabs.hpp> //#include <windows.h>
#undef NOMINMAX

namespace mccinfo {
namespace fsm2 {
template <typename T, std::size_t N>
class predicate_sequence {
public:
    template <typename... Predicates>
    constexpr predicate_sequence(Predicates... preds) : _seq{ preds... }, current_index(0) {
        static_assert(sizeof...(preds) == N, "The number of predicates must match the template parameter N.");
    }
    constexpr predicate_sequence() : _seq{}, current_index(0) {
        static_assert(N == 0, "This constructor can only be used when N is 0.");
    }
    bool is_complete() const { return current_index == N; }
    bool try_advance(const EVENT_RECORD& record, const krabs::trace_context& trace_context) const {
        if (!is_complete()) {
            if ((*_seq[current_index])(record, trace_context)) {
                current_index += 1;
                return true;
            }
        }
        return false;
    }
    T consume_next_event() const {
        if (is_complete()) {
            current_index = 0;
            return T();
        }
        else {
            throw std::runtime_error("big no no");
        }
    }
private:
    std::array<krabs::predicates::details::predicate_base*, N> _seq;
    mutable size_t current_index{0};
};

template <typename T, typename... Predicates>
predicate_sequence(T, Predicates...) -> predicate_sequence<T, sizeof...(Predicates)>;

template <typename T, typename... Predicates>
constexpr auto make_predicate_sequence(Predicates... preds) {
    return predicate_sequence<T, sizeof...(preds)>{preds...};
}

} // namespace fsm2
} // namespace mccinfo