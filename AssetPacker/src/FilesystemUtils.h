#pragma once

#include <filesystem>

namespace AssetPacker::FilesystemUtils {
    std::filesystem::path getFilePathOfFileRelativeToConfigFile(const std::filesystem::path& settingsFilePath, const std::filesystem::path& relativeFilePath);
    bool checkFileExists(const std::filesystem::path& path);
    std::string readFileToString(const std::filesystem::path& path);
    std::vector<uint8_t> readFileBinary(const std::filesystem::path& path);
    std::string normalizeFilePath(std::string path);

}

