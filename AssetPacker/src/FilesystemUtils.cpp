#include "FilesystemUtils.h"
#include "AssetPackerLogging.h"
#include <fstream>
#include <sstream>

namespace AssetPacker::FilesystemUtils {
    std::filesystem::path getFilePathOfFileRelativeToConfigFile(const std::filesystem::path& settingsFilePath, const std::filesystem::path& relativeFilePath) {
        std::filesystem::path settingsDir = settingsFilePath.parent_path();
        std::filesystem::path fullPath = settingsDir / relativeFilePath;
        try {
            return std::filesystem::canonical(fullPath);
        } catch (const std::filesystem::filesystem_error& e) {
            AP_LOGGING_ERROR("Error resolving path '" + fullPath.string() + "': " + e.what())
            exit(1);
        }
    }

    bool checkFileExists(const std::filesystem::path& path) {
        return std::filesystem::exists(path);
    }

    std::string readFileToString(const std::filesystem::path& path) {
        std::ifstream file(path);

        if (file.is_open()) {
            std::stringstream stringStream;
            stringStream << file.rdbuf();
            std::string out = stringStream.str();
            file.close();
            return out;
        }

        AP_LOGGING_ERROR("Failed to open file for reading: " + path.string())
        return "";
    }

    std::vector<uint8_t> readFileBinary(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            AP_LOGGING_ERROR("Unable to load File: {0}", path.string());
            return {};
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<uint8_t> buffer(fileSize);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        return buffer;
    }

    std::string normalizeFilePath(std::string path) {
        // Replace backslashes with slashes
        std::replace(path.begin(), path.end(), '\\', '/');

        // Remove leading "./"
        if (path.rfind("./", 0) == 0) { // starts with "./"
            path.erase(0, 2);
        }

        // Collapse multiple consecutive '/'
        std::string result;
        result.reserve(path.size());
        bool prevSlash = false;
        for (char c : path) {
            if (c == '/') {
                if (!prevSlash) result += c;
                prevSlash = true;
            } else {
                result += c;
                prevSlash = false;
            }
        }

        return result;
    }

}

