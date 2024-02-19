#include "mccinfo/core/log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace mccinfo::core
{
	std::shared_ptr<spdlog::logger> log::s_CoreLogger;

	void log::init() {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("mccinfo.log", true));
        sinks[0]->set_pattern("%^[%T] %n: %v%$");
        sinks[1]->set_pattern("[%T] [%l] %n: %v");

        s_CoreLogger = std::make_shared<spdlog::logger>("[MCCINFO]", std::begin(sinks), std::end(sinks));
        spdlog::register_logger(s_CoreLogger);
        s_CoreLogger->set_level(spdlog::level::trace);
        s_CoreLogger->flush_on(spdlog::level::trace);
	}
}