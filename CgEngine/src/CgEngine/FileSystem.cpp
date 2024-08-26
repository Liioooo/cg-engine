#include "FileSystem.h"
#include <fstream>
#include "Logging.h"

namespace CgEngine {
    std::string FileSystem::readFileToString(const std::string& path) {
        std::ifstream file(path);

        if (file.is_open()) {
            std::stringstream stringStream;
            stringStream << file.rdbuf();
            std::string out = stringStream.str();
            file.close();
            return out;
        }

        CG_LOGGING_ERROR("Unable to load File: {0}", path);
        return std::string{};
    }

    bool FileSystem::checkFileExists(const std::string& path) {
        std::ifstream file(path);
        if (file.is_open()) {
            file.close();
            return true;
        }
        return false;
    }

    std::string FileSystem::getAsGamePath(const std::string& path) {
        if (path.rfind("assets/game/", 0) == 0) {
            return path;
        }

        return "assets/game/" + path;
    }

    std::string FileSystem::getAsEnginePath(const std::string& path) {
        if (path.rfind("assets/engine/", 0) == 0) {
            return path;
        }

        return "assets/engine/" + path;
    }
}
