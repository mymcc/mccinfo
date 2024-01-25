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
    context(callback_table& cbtable) : sm_(cbtable) {}

    void start() {
        dispatch_thread_ = std::thread([&]{
            provider_.enable_dispatch_to(&sm_);
            provider_.start();
        });
    }
    void stop() {
        if (dispatch_thread_.joinable()) {
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