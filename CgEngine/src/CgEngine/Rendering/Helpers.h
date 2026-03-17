#pragma once

#include "CgEngineSharedUtils/Enums.h"

namespace CgEngine {

    namespace Helpers {
        inline int getSizeForShaderDataType(ShaderDataType type) {
            switch (type) {
                case ShaderDataType::Float:
                    return 4;
                case ShaderDataType::Float2:
                    return 4 * 2;
                case ShaderDataType::Float3:
                    return 4 * 3;
                case ShaderDataType::Float4:
                    return 4 * 4;
                case ShaderDataType::Mat3:
                    return 4 * 3 * 3;
                case ShaderDataType::Mat4:
                    return 4 * 4 * 4;
                case ShaderDataType::Int:
                    return 4;
                case ShaderDataType::Int2:
                    return 4 * 2;
                case ShaderDataType::Int3:
                    return 4 * 3;
                case ShaderDataType::Int4:
                    return 4 * 4;
                case ShaderDataType::Bool:
                    return 1;
            }
            return 0;
        }

        std::tuple<unsigned char*, int, int> loadImageData(const std::filesystem::path& path);
        void freeImageData(unsigned char* data);
        uint32_t calculateMipCount(uint32_t width, uint32_t height);
        std::vector<uint8_t> loadShaderBinaryWithType(const std::string& name, const std::string& type, ShaderEnv env);
        std::vector<uint8_t> loadShaderBinary(const std::string& name, ShaderEnv env);
    }
}
