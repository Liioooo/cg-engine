#include "AssetPackerLogging.h"

namespace AssetPacker {

    void AssetPackerLogging::init() {
        apLogger = spdlog::stdout_color_mt("AssetPacker");
        apLogger->set_level(spdlog::level::debug);
        apLogger->set_pattern("[%H:%M:%S][%n][%l]: %v");
    }

}
