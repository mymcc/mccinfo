#pragma once

namespace mccinfo {
namespace fsm {
namespace events {
struct event_base {};

struct launcher_start : public event_base {};
struct launcher_terminate : public event_base {};
struct mcc_start : public event_base{};
struct mcc_terminate : public event_base{};

} //namespace events
} // namespace fsm
} // namespace mccinfo