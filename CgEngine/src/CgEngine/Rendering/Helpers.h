#pragma once

#include "CgEngineSharedUtils/Enums.h"

namespace CgEngine {

    namespace Helpers {
        inline int getSizeForShaderDataType(const ShaderDataType type) {
            switch (type) {
                case ShaderDataType::Float:
                    return 4;
                case ShaderDataType::Float2:
                    return 4 * 2;
                case ShaderDataType::Float3:
                    return 4 * 3;
                case ShaderDataType::Float4:
                    return 4 * 4;
                case ShaderDataType::Int:
                    return 4;
                case ShaderDataType::Int2:
                    return 4 * 2;
                case ShaderDataType::Int3:
                    return 4 * 3;
                case ShaderDataType::Int4:
                    return 4 * 4;
            }
            return 0;
        }

        inline uint32_t getBytesPerPixelForTextureFormat(const TextureFormat format) {
            switch (format) {
                case TextureFormat::R:                  return 1;
                case TextureFormat::RG:                 return 2;
                case TextureFormat::RGBA:               return 4;
                case TextureFormat::RGBA_SRGB:          return 4;

                case TextureFormat::RedFloat16:         return 2;
                case TextureFormat::RedFloat32:         return 4;

                case TextureFormat::RedGreenFloat16:    return 4; // 2 * 16-bit
                case TextureFormat::RedGreenFloat32:    return 8; // 2 * 32-bit

                case TextureFormat::Float16A:           return 8;  // RGBA16F
                case TextureFormat::Float32A:           return 16; // RGBA32F
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
