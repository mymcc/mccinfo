#pragma once

#include "state.hpp"
#include "..\..\utility.hpp"
#include "..\events\events.hpp"
#include "..\static_predicates.hpp"
namespace mccinfo {
namespace fsm {
namespace states {

inline constinit auto seq1 =
    fsm::predicates::details::make_sequence(&fsm2::predicates::launcher_start_pred);

inline constinit auto seq2 =
    fsm::predicates::details::make_sequence(&fsm2::predicates::launcher_end_pred);

inline constexpr auto off_edges = edges::make_edges(
    std::make_tuple(&seq1, events::launcher_start{}),
    std::make_tuple(&seq2, events::launcher_terminate{})
);

struct off : public state<off> {
    static constexpr auto edges = edges::make_edges(
        std::make_tuple(&seq1, events::launcher_start{}), 
        std::make_tuple(&seq2, events::launcher_terminate{})
    );
};

struct launching : public state<launching> {
    using edges_ = int;

  public:
    static constexpr bool edges{};

};

struct on : public state<on> {
    using edges_ = int;
  public:
    static constexpr bool edges{};

};
} // namespace states
} // namespace fsm
} // namespace mccinfo