#include <FileSystem.h>
#include <Asserts.h>
#include "Helpers.h"

#include "GraphicsObjectsFactory.h"
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

        std::vector<uint8_t> loadShaderBinaryWithType(const std::string& name, const std::string& type, ShaderEnv env) {
            return loadShaderBinary(name + "_" + type + ".spv", env);
        }

        std::vector<uint8_t> loadShaderBinary(const std::string& name, ShaderEnv env) {
            std::filesystem::path path;

            if (GraphicsObjectsFactory::getGraphicsAPI() == GraphicsAPI::OpenGL) {
                path = env == ShaderEnv::Engine ? FileSystem::getAsEnginePath(std::filesystem::path("compiled_shaders/opengl") / name) : FileSystem::getAsGamePath(std::filesystem::path("compiled_shaders/opengl") / name);
            } else {
                path = env == ShaderEnv::Engine ? FileSystem::getAsEnginePath(std::filesystem::path("compiled_shaders/vulkan") / name) : FileSystem::getAsGamePath(std::filesystem::path("compiled_shaders/vulkan") / name);
            }

            if (!FileSystem::checkFileExists(path)) {
                return {};
            }
            return FileSystem::readFileBinary(path);
        }
    }

}
