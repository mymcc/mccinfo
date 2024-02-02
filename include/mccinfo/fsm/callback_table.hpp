#pragma once
#include <functional>
#include <iostream>
#include <deque>
namespace mccinfo {
namespace fsm {

#define BITFLAG(x) \
    (1ULL << x)

#define MASK_BITS_UP_TO(x) \
    ((1ULL << (x + 1)) - 1)


enum control_flags : uint64_t {
    ON_STATE_ENTRY = BITFLAG(0),
    ON_STATE_EXIT = BITFLAG(1),
};

enum state_flags : uint64_t {
    OFF                 = BITFLAG(2),
    LAUNCHING           = BITFLAG(3),
    ON                  = BITFLAG(4),
    OFFLINE             = BITFLAG(5),
    WAITING_ON_LAUNCH   = BITFLAG(6),
    IDENTIFYING_SESSION = BITFLAG(7),
    IN_MENUS            = BITFLAG(8),
    LOADING_IN          = BITFLAG(9),
    IN_GAME             = BITFLAG(10),
    LOADING_OUT         = BITFLAG(11),
    NONE                = BITFLAG(12),
    HALOCE              = BITFLAG(13),
    HALO2               = BITFLAG(14),
    HALO2A              = BITFLAG(15),
    HALO3               = BITFLAG(16),
    HALO3ODST           = BITFLAG(17),
    HALO4               = BITFLAG(18),
    HALOREACH           = BITFLAG(19),
};


class callback_table {
    using CBTableType_ = 
        std::unordered_map<uint64_t, std::deque<std::function<void()>>>;

  public:
    void execute_callback(uint64_t key) {
        if (auto it = table_.find(key); it != table_.end()) {
            for (auto &cb : it->second) {
                cb();
            }
        }
    }

    void add_callback(uint64_t flags, std::function<void()> cb) {
        
        // on_state_entry
        for (size_t i = 2; i < 20; ++i) {
            uint64_t key = flags & (ON_STATE_ENTRY | BITFLAG(i));
            if (auto it = table_.find(key); it != table_.end()) {
                it->second.emplace_back(cb);
            }
        }

        // on_state_exit
        for (size_t i = 2; i < 20; ++i) {
            uint64_t key = flags & (ON_STATE_EXIT | BITFLAG(i));
            if (auto it = table_.find(key); it != table_.end()) {
                it->second.emplace_back(cb);
            }
        }
    }

  private:
    std::function<void()> cb_;
    CBTableType_ table_{
        {ON_STATE_ENTRY | OFF, {}},
        {ON_STATE_ENTRY | LAUNCHING, {}},
        {ON_STATE_ENTRY | ON, {}},
        {ON_STATE_ENTRY | OFFLINE, {}},
        {ON_STATE_ENTRY | WAITING_ON_LAUNCH, {}},
        {ON_STATE_ENTRY | IN_MENUS, {}},
        {ON_STATE_ENTRY | LOADING_IN, {}},
        {ON_STATE_ENTRY | IN_GAME, {}},
        {ON_STATE_ENTRY | LOADING_OUT, {}},
        {ON_STATE_ENTRY | NONE, {}},
        {ON_STATE_ENTRY | HALOCE, {}},
        {ON_STATE_ENTRY | HALO2, {}},
        {ON_STATE_ENTRY | HALO2A, {}},
        {ON_STATE_ENTRY | HALO3, {}},
        {ON_STATE_ENTRY | HALO3ODST, {}},
        {ON_STATE_ENTRY | HALO4, {}},
        {ON_STATE_ENTRY | HALOREACH, {}},

        {ON_STATE_EXIT | OFF, {}},
        {ON_STATE_EXIT | LAUNCHING, {}},
        {ON_STATE_EXIT | ON, {}},
        {ON_STATE_EXIT | OFFLINE, {}},
        {ON_STATE_EXIT | WAITING_ON_LAUNCH, {}},
        {ON_STATE_EXIT | IN_MENUS, {}},
        {ON_STATE_EXIT | LOADING_IN, {}},
        {ON_STATE_EXIT | IN_GAME, {}},
        {ON_STATE_EXIT | LOADING_OUT, {}},
        {ON_STATE_EXIT | NONE, {}},
        {ON_STATE_EXIT | HALOCE, {}},
        {ON_STATE_EXIT | HALO2, {}},
        {ON_STATE_EXIT | HALO2A, {}},
        {ON_STATE_EXIT | HALO3, {}},
        {ON_STATE_EXIT | HALO3ODST, {}},
        {ON_STATE_EXIT | HALO4, {}},
        {ON_STATE_EXIT | HALOREACH, {}}
    };
};


#undef BITFLAG
#undef MASK_BITS_UP_TO
}
}