#include <FileSystem.h>
#include <Asserts.h>
#include "Helpers.h"
#include "stbi_image.h"

namespace CgEngine {

    namespace Helpers {
        std::tuple<unsigned char*, int, int> loadImageData(const std::filesystem::path& path) {
            int loadWidth, loadHeight, channels;
            auto data = stbi_load(path.string().c_str(), &loadWidth, &loadHeight, &channels, STBI_rgb_alpha);
            return {data, loadWidth,loadHeight};
        }

        void freeImageData(unsigned char* data) {
            stbi_image_free(data);
        }

        uint32_t calculateMipCount(uint32_t width, uint32_t height) {
            return static_cast<uint32_t>(std::floor(std::log2(glm::min(width, height))) + 1);
        }

        std::vector<char> loadShaderBinaryWithType(const std::string& name, const std::string& type, ShaderEnv env) {
            return Helpers::loadShaderBinary(name + "_" + type + ".spv", env);
        }

        std::vector<char> loadShaderBinary(const std::string& name, ShaderEnv env) {
            const std::filesystem::path path = env == ShaderEnv::Engine ? FileSystem::getAsEnginePath(std::filesystem::path("compiled_shaders/opengl") / name) : FileSystem::getAsGamePath(std::filesystem::path("compiled_shaders/opengl") / name);

            if (!FileSystem::checkFileExists(path)) {
                return {};
            }
            return FileSystem::readFileBinary(path);
        }
    }

}
