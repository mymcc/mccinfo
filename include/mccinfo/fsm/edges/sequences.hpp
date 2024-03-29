#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <krabs/krabs.hpp> //#include <windows.h>
#undef NOMINMAX

#include <array>
#include <optional>

namespace mccinfo {
namespace fsm {
namespace edges {
namespace details {

enum sequence_policy {
    weak = 0,
    strict = 1,
};

struct sequence_base {
    virtual bool try_advance(const EVENT_RECORD &record,
                             const krabs::trace_context &trace_context) const = 0;
    virtual bool is_complete() const = 0;
    virtual void reset() = 0;
};

template <std::size_t N> struct sequence : public sequence_base {
  public:
    template <typename... predicates>
    constexpr sequence(
        const sequence_policy &pol, predicates... preds) : seq_{preds...}, pol_{pol} {
        static_assert(sizeof...(preds) == N,
                      "The number of predicates must match the template parameter N.");
    }

    sequence(const sequence &other) : current_(0), seq_(other.seq_), pol_(other.pol_) {
    }

    sequence &operator=(const sequence &other) {
        if (this != &other) {
            current_ = 0;
            seq_ = other.seq_;
            pol_ = other.pol_;
        }
        return *this;
    }

    sequence(sequence &&) = delete;
    sequence &operator=(sequence &&) = delete;

    virtual bool try_advance(const EVENT_RECORD &record,
                             const krabs::trace_context &trace_context) const override {
        if (!is_complete()) {
            if ((*seq_[current_])(record, trace_context)) {
                current_ += 1;
                return true;
            } 
            else if (pol_ == sequence_policy::strict) {
                current_ = 0;
            }
        }
        return false;
    }

    virtual bool is_complete() const override {
        return current_ == N;
    }

    virtual void reset() override {
        current_ = 0;
    }

  private:
    sequence_policy pol_{};
    mutable size_t current_{0};
    std::array<krabs::predicates::details::predicate_base *, N> seq_;
};
template <typename... predicates> sequence(predicates...) -> sequence<sizeof...(predicates)>;

} // details

template <typename... predicates>
constexpr auto make_sequence(predicates&&... preds) {
	return details::sequence<sizeof...(preds)>{details::sequence_policy::weak, std::forward<predicates>(preds)...};
}

template <typename policy, typename... predicates>
constexpr auto make_sequence_with_policy(const policy& pol, predicates&&... preds) {
    return details::sequence<sizeof...(preds)>(pol, std::forward<predicates>(preds)...);
}

}
}
}