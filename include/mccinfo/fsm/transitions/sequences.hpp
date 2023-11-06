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
namespace predicates {
namespace details {

	struct sequence_base {
		virtual bool try_advance(const EVENT_RECORD &record, const krabs::trace_context &trace_context) const = 0;
        virtual bool is_complete() const = 0;
	};

	template <typename event, std::size_t N>
	struct sequence : public sequence_base {
      public:
		  template <typename... predicates> 
		  constexpr sequence(predicates... preds) : _seq{preds...} {
            static_assert(sizeof...(preds) == N,
                          "The number of predicates must match the template parameter N.");
		  }

          virtual bool try_advance(const EVENT_RECORD &record,
                                   const krabs::trace_context &trace_context) const override {
            return (is_complete()) ? false : (*_seq[current++])(record, trace_context);
		  }

          virtual bool is_complete() const override {
              return current == N;
          }

        private: 
		  mutable size_t current{0};
		  std::array<krabs::predicates::details::predicate_base*, N> _seq;
	};

	template <typename event, typename... predicates>
	sequence(event, predicates...) -> sequence<event, sizeof...(predicates)>;

	template <typename event, typename... predicates>
	constexpr auto make_sequence(predicates... preds) {
		return sequence<event, sizeof...(preds)>{preds...};
	}
} // details


}
}
}