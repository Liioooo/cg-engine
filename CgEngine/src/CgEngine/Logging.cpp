#include "Logging.h"

namespace CgEngine {
    std::shared_ptr<spdlog::logger> Logging::cgLogger;

    void Logging::init() {
        cgLogger = spdlog::stdout_color_mt("CG");
        cgLogger->set_level(spdlog::level::debug);
        cgLogger->set_pattern("[%H:%M:%S][%n][%l]: %v");
    }
}
