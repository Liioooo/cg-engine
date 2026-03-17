#pragma once

#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace AssetPacker {

    class AssetPackerLogging {
    public:
        static void init();

        inline static std::shared_ptr<spdlog::logger> getLogger() {
            return apLogger;
        };
    private:
        static inline std::shared_ptr<spdlog::logger> apLogger;
    };
}

#define AP_LOGGING_INFO(...) ::AssetPacker::AssetPackerLogging::getLogger()->info(__VA_ARGS__);
#define AP_LOGGING_WARNING(...) ::AssetPacker::AssetPackerLogging::getLogger()->warn(__VA_ARGS__);
#define AP_LOGGING_ERROR(...) ::AssetPacker::AssetPackerLogging::getLogger()->error(__VA_ARGS__);
