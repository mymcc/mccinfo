#pragma once
namespace mccinfo {
namespace fsm2 {
namespace events {
struct start {};
struct launcher_start {};
struct launcher_failed {};
struct launcher_end {};
struct process_found {};
struct process_lost {};
struct reset {};

struct launch_start {};
struct eac {};
struct no_eac {};
struct launch_reset {};

struct process_event {};
}
}
}