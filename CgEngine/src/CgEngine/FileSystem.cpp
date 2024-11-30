#include "FileSystem.h"
#include <fstream>
#include "Logging.h"

namespace CgEngine {
    std::string FileSystem::readFileToString(const std::filesystem::path& path) {
        std::ifstream file(path);

        if (file.is_open()) {
            std::stringstream stringStream;
            stringStream << file.rdbuf();
            std::string out = stringStream.str();
            file.close();
            return out;
        }

        CG_LOGGING_ERROR("Unable to load File: {0}", path.string());
        return "";
    }

    bool FileSystem::checkFileExists(const std::filesystem::path& path) {
        return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
    }

    std::filesystem::path FileSystem::getAsGamePath(const std::filesystem::path& path) {
        if (!isSubpath(path, relativeGame)) {
            return relativeGame / path;
        }
        return path;
    }

    std::filesystem::path FileSystem::getAsEnginePath(const std::filesystem::path& path) {
        if (!isSubpath(path, relativeGame)) {
            return relativeEngine / path;
        }
        return path;
    }

    bool FileSystem::isSubpath(const std::filesystem::path& path, const std::filesystem::path& base) {
        auto rel = std::filesystem::relative(path, base);
        return !rel.empty() && rel.native()[0] != '.';
    }

    std::string FileSystem::getExtension(const std::filesystem::path& path) {
        return path.extension().string();
    }
}
