#pragma once
#include <cstddef>
#include <krabs/krabs.hpp>

template <std::size_t N>
struct predicate_container {
	using sequence = std::array<std::shared_ptr<krabs::predicates::details::predicate_base>, N>;
};

template <typename... Ts> auto make_predicate_sequence(Ts &&...args) {
	return typename predicate_container<sizeof...(Ts)>::sequence {
		std::forward<Ts>(args)...
	};
}