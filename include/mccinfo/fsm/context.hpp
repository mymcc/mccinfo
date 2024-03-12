#pragma once

#include "mccinfo/fsm/controller.hpp"
#include "mccinfo/fsm/provider.hpp"
#include "lockfree/lockfree.hpp"
#include "mccinfo/fsm/callback_table.hpp"

#include <iostream>
#include <thread>

namespace mccinfo {
namespace fsm {

class context {
  public:
    context(callback_table& cbtable) : sm_(cbtable) {
        MI_CORE_TRACE("Constructing fsm context ...");
    }

    std::string get_map_info() const {
        return sm_.get_map_info();
    }

    std::vector<std::optional<mccinfo::query::MCCInstallInfo>> get_install_info() const {
        return sm_.get_install_info();
    }

    std::optional<mccinfo::query::MCCInstallInfo> get_active_install_info() const {
        return std::nullopt;
    }

    const extended_match_info &get_extended_match_info() const {
        return sm_.get_extended_match_info();
    }

    void start() {
        dispatch_thread_ = std::thread([&]{
            MI_CORE_TRACE("Starting fsm context ...");

            provider_.enable_dispatch_to(&sm_);
            provider_.start();
        });
    }
    void stop() {
        if (dispatch_thread_.joinable()) {
            MI_CORE_TRACE("Stopping fsm context ...");

            provider_.stop();
            dispatch_thread_.join();
        }
    }
    
  private:
      controller<> sm_;
      event_provider provider_{};
      
      
      bool stop_ = false;
      std::mutex mut_;
      std::thread dispatch_thread_;
};

}
}