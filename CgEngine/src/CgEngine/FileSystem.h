#pragma once

namespace CgEngine {

    class FileSystem {
    public:
        static std::string readFileToString(const std::filesystem::path& path);
        static std::vector<uint8_t> readFileBinary(const std::filesystem::path& path);
        static bool checkFileExists(const std::filesystem::path& path);

        static std::filesystem::path getAsGamePath(const std::filesystem::path& path);
        static std::filesystem::path getAsEnginePath(const std::filesystem::path& path);

        static bool isSubpath(const std::filesystem::path& path, const std::filesystem::path& base);
        static std::string getExtension(const std::filesystem::path& path);

    private:
        static inline std::filesystem::path relativeGame = "./assets/game/";
        static inline std::filesystem::path relativeEngine = "./assets/engine/";
    };

}
