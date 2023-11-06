#pragma once
namespace mccinfo {
namespace fsm {
namespace events {
struct launcher_start {bool valid{};};
struct launcher_terminate {bool valid{};};
struct mcc_start {bool valid{};};
struct mcc_terminate {bool valid{};};
} //namespace events
} // namespace fsm
} // namespace mccinfo