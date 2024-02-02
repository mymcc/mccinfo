#pragma once

namespace mccinfo {
namespace fsm {
namespace events {

struct load_start{};
struct match_start{};
struct match_abort{};
struct match_end{};
struct unload_end{};
struct in_menus_identified{};
struct match_found{};
struct match_paused{};

} // namespace events
} // namespace fsm
} // namespace mccinfo