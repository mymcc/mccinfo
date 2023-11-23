#pragma once

#include "state.hpp"

namespace mccinfo {
namespace fsm {
namespace states {
struct eac_off : public state<eac_off> {
    using edges_ = int;

  public:
    static constexpr bool edges{};

};
struct eac_on : public state<eac_on> {
    using edges_ = int;

  public:
    static constexpr bool edges{};


};
}
} // namespace fsm
} // namespace mccinfo