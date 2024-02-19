#pragma once

#pragma warning(push, 0)

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)


namespace mccinfo::core 
{
class log {
  public:
    static void init();

    static std::shared_ptr<spdlog::logger> &GetCoreLogger() {
        return s_CoreLogger;
    }
    //static std::shared_ptr<spdlog::logger> &GetClientLogger() {
    //    return s_ClientLogger;
    //}

  private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    //static std::shared_ptr<spdlog::logger> s_ClientLogger;
};

}

#define MI_CORE_TRACE(...)      ::mccinfo::core::log::GetCoreLogger()->trace(__VA_ARGS__)
#define MI_CORE_INFO(...)       ::mccinfo::core::log::GetCoreLogger()->info(__VA_ARGS__)
#define MI_CORE_WARN(...)       ::mccinfo::core::log::GetCoreLogger()->warn(__VA_ARGS__)
#define MI_CORE_ERROR(...)      ::mccinfo::core::log::GetCoreLogger()->error(__VA_ARGS__)
#define MI_CORE_CRITICAL(...)   ::mccinfo::core::log::GetCoreLogger()->critical(__VA_ARGS__)