#pragma once

#include <Asserts.h>
#include "OpenGLHelpers.h"
#include "glad/glad.h"
#include "Rendering/Helpers.h"

namespace CgEngine {

    namespace OpenGLHelpers {
        unsigned int shaderDataTypeToOpenGLBaseType(ShaderDataType type) {
            switch (type) {
                case ShaderDataType::Float:
                case ShaderDataType::Float2:
                case ShaderDataType::Float3:
                case ShaderDataType::Float4:
                case ShaderDataType::Mat3:
                case ShaderDataType::Mat4: {
                    return GL_FLOAT;
                }
                case ShaderDataType::Int:
                case ShaderDataType::Int2:
                case ShaderDataType::Int3:
                case ShaderDataType::Int4: {
                    return GL_INT;
                }
                case ShaderDataType::Bool: {
                    return GL_BOOL;
                }
            }
            return 0;
        }

        int depthCompareOperatorToOpenGL(DepthCompareOperator op) {
            switch (op) {
                case DepthCompareOperator::Never:
                    return GL_NEVER;
                case DepthCompareOperator::Less:
                    return GL_LESS;
                case DepthCompareOperator::Equal:
                    return GL_EQUAL;
                case DepthCompareOperator::LessOrEqual:
                    return GL_LEQUAL;
                case DepthCompareOperator::Greater:
                    return GL_GREATER;
                case DepthCompareOperator::NotEqual:
                    return GL_NOTEQUAL;
                case DepthCompareOperator::GreaterOrEqual:
                    return GL_GEQUAL;
                case DepthCompareOperator::Always:
                    return GL_ALWAYS;
            }
            return GL_LESS;
        }

        int blendingEquationToOpenGL(BlendingEquation eq) {
            switch (eq) {
                case BlendingEquation::Add:
                    return GL_FUNC_ADD;
                case BlendingEquation::Subtract:
                    return GL_FUNC_SUBTRACT;
                case BlendingEquation::ReverseSubtract:
                    return GL_FUNC_REVERSE_SUBTRACT;
                case BlendingEquation::Min:
                    return GL_MIN;
                case BlendingEquation::Max:
                    return GL_MAX;
            }
            return GL_FUNC_ADD;
        }

        int blendingFunctionToOpenGL(BlendingFunction fn) {
            switch (fn) {
                case BlendingFunction::Zero:
                    return GL_ZERO;
                case BlendingFunction::One:
                    return GL_ONE;
                case BlendingFunction::SrcColor:
                    return GL_SRC_COLOR;
                case BlendingFunction::OneMinusSrcColor:
                    return GL_ONE_MINUS_SRC_COLOR;
                case BlendingFunction::SrcAlpha:
                    return GL_SRC_ALPHA;
                case BlendingFunction::OneMinusSrcAlpha:
                    return GL_ONE_MINUS_SRC_ALPHA;
                case BlendingFunction::DestAlpha:
                    return GL_DST_ALPHA;
                case BlendingFunction::OneMinusDestAlpha:
                    return GL_ONE_MINUS_DST_ALPHA;
                case BlendingFunction::DestColor:
                    return GL_DST_COLOR;
                case BlendingFunction::OneMinusDestColor:
                    return GL_ONE_MINUS_DST_COLOR;
            }
            return GL_ZERO;
        }

        int getOpenGLTextureInternalFormat(TextureFormat format, bool compression) {
            if (!compression) {
                switch (format) {
                    case TextureFormat::R:               return GL_RED;
                    case TextureFormat::RedFloat16:      return GL_R16F;
                    case TextureFormat::RedFloat32:      return GL_R32F;
                    case TextureFormat::RedGreenFloat16: return GL_RG16F;
                    case TextureFormat::RedGreenFloat32: return GL_RG32F;
                    case TextureFormat::RGB:             return GL_RGB;
                    case TextureFormat::RGBA:            return GL_RGBA;
                    case TextureFormat::Float16A:        return GL_RGBA16F;
                    case TextureFormat::Float32A:        return GL_RGBA32F;
                    case TextureFormat::Float16:         return GL_RGB16F;
                    case TextureFormat::Float32:         return GL_RGB32F;
                }
                return 0;
            }
            switch (format) {
                case TextureFormat::R:               return GL_COMPRESSED_RED;
                case TextureFormat::RGB:             return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                case TextureFormat::RGBA:            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                case TextureFormat::Float16:         return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
                case TextureFormat::Float32:         return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
            }
            CG_ASSERT(false, "Texture Compression was enabled with an unsupported Texture format")
            return 0;
        }

        int getOpenGLTextureType(TextureFormat format) {
            if (format == TextureFormat::RGB || format == TextureFormat::RGBA || format == TextureFormat::R) {
                return GL_UNSIGNED_BYTE;
            }
            return GL_FLOAT;
        }

        int getOpenGLTextureFormat(TextureFormat format) {
            if (format == TextureFormat::RGBA || format == TextureFormat::Float16A || format == TextureFormat::Float32A) {
                return GL_RGBA;
            } else if (format == TextureFormat::R || format == TextureFormat::RedFloat16 || format == TextureFormat::RedFloat32) {
                return GL_RED;
            } else if (format == TextureFormat::RedGreenFloat16 || format == TextureFormat::RedGreenFloat32) {
                return GL_RG;
            }
            return GL_RGB;
        }

        int getOpenGLWrapMode(TextureWrap wrap) {
            switch (wrap) {
                case TextureWrap::Repeat:       return GL_REPEAT;
                case TextureWrap::Clamp:        return GL_CLAMP_TO_EDGE;
                case TextureWrap::ClampBorder:  return GL_CLAMP_TO_BORDER;
            }
            return 0;
        }

        void applyMipMapFiltering(MipMapFiltering mipMapFiltering, unsigned int textureType) {
            switch (mipMapFiltering) {
                case MipMapFiltering::Nearest: {
                    glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    return;
                }
                case MipMapFiltering::Bilinear: {
                    glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    return;
                }
                case MipMapFiltering::Trilinear: {
                    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    return;
                }
            }
        }

        int attachmentTypeToOpenGLInternalFormat(AttachmentType type) {
            switch (type) {
                case AttachmentType::RGBA8:
                    return GL_RGBA8;
                case AttachmentType::RGBA16F:
                    return GL_RGBA16F;
                case AttachmentType::RG8:
                    return GL_RG8;
                case AttachmentType::RG16F:
                    return GL_RG16F;
            }
            CG_LOGGING_ERROR("Given attachment type is not a color attachment!")
            return GL_RGBA8;
        }

        int shaderImageAccessToOpenGL(ShaderImageAccess access) {
            switch (access) {
                case ShaderImageAccess::ReadOnly:
                    return GL_READ_ONLY;
                case ShaderImageAccess::WriteOnly:
                    return GL_WRITE_ONLY;
                case ShaderImageAccess::ReadWrite:
                    return GL_READ_WRITE;
            }
            return GL_READ_ONLY;
        }

        int getOpenGLTextureFormatForImageBind(TextureFormat format) {
            if (format == TextureFormat::Float32 || format == TextureFormat::Float32A) {
                return GL_RGBA32F;
            }
            if (format == TextureFormat::Float16 || format == TextureFormat::Float16A) {
                return GL_RGBA16F;
            }
            if (format == TextureFormat::RedFloat16) {
                return GL_R16F;
            }
            if (format == TextureFormat::RedFloat32) {
                return GL_R32F;
            }
            if (format == TextureFormat::RedGreenFloat16) {
                return GL_RG16F;
            }
            if (format == TextureFormat::RedGreenFloat32) {
                return GL_RG32F;
            }
            if (format == TextureFormat::R) {
                return GL_R8;
            }
            return GL_RGBA8;
        }

        uint32_t loadOpenGLGraphicsShader(const std::string& name, ShaderEnv env) {
            CG_LOGGING_DEBUG("Loading Shader: {0}", name)

            std::vector<char> vertexSource = Helpers::loadShaderBinaryWithType(name, "vertex", env);
            std::vector<char> fragmentSource = Helpers::loadShaderBinaryWithType(name, "fragment", env);
            std::vector<char> geometrySource = Helpers::loadShaderBinaryWithType(name, "geometry", env);
            std::vector<char> tcsSource = Helpers::loadShaderBinaryWithType(name, "tcs", env);
            std::vector<char> tesSource = Helpers::loadShaderBinaryWithType(name, "tes", env);

            uint32_t handle = glCreateProgram();
            bool error = false;

            if (!vertexSource.empty()) {
                error |= OpenGLHelpers::createShaderType(GL_VERTEX_SHADER, "VERTEX", vertexSource, handle);
            }
            if (!fragmentSource.empty()) {
                error |= OpenGLHelpers::createShaderType(GL_FRAGMENT_SHADER, "FRAGMENT", fragmentSource, handle);
            }
            if (!geometrySource.empty()) {
                error |= OpenGLHelpers::createShaderType(GL_GEOMETRY_SHADER, "GEOMETRY", geometrySource, handle);
            }
            if (!tcsSource.empty()) {
                error |= OpenGLHelpers::createShaderType(GL_TESS_CONTROL_SHADER, "TCS", tcsSource, handle);
            }
            if (!tesSource.empty()) {
                error |= OpenGLHelpers::createShaderType(GL_TESS_EVALUATION_SHADER, "TES", tesSource, handle);
            }

            glLinkProgram(handle);
            error |= OpenGLHelpers::checkShaderErrors(handle, "PROGRAM");

            if (error) {
                glDeleteProgram(handle);
                CG_LOGGING_ERROR("Failed to load shader: {0}", name)
                return ~0;
            }

            CG_LOGGING_DEBUG("Loaded Shader: {0}", name)
            return handle;
        }

        uint32_t loadOpenGLComputeShader(const std::string& name, ShaderEnv env) {
            CG_LOGGING_DEBUG("Loading ComputeShader: {0}", name)

            std::vector<char> source = Helpers::loadShaderBinaryWithType(name, "compute", env);

            uint32_t handle = glCreateProgram();
            bool error = OpenGLHelpers::createShaderType(GL_COMPUTE_SHADER, "COMPUTE", source, handle);

            glLinkProgram(handle);
            error |= OpenGLHelpers::checkShaderErrors(handle, "PROGRAM");

            if (error) {
                glDeleteProgram(handle);
                CG_LOGGING_ERROR("Failed to load compute shader: {0}", name)
                return ~0;
            }

            CG_LOGGING_DEBUG("Loaded ComputeShader: {0}", name)
            return handle;
        }

        bool checkShaderErrors(uint32_t shaderHandle, const std::string& type) {
            int isCompiled;
            int maxLength;

            if (type == "PROGRAM") {
                glGetProgramiv(shaderHandle, GL_LINK_STATUS, &isCompiled);
                if (!isCompiled) {
                    glGetProgramiv(shaderHandle, GL_INFO_LOG_LENGTH, &maxLength);
                    char* infoLog = new char[maxLength];
                    glGetProgramInfoLog(shaderHandle, maxLength, &maxLength, infoLog);
                    CG_LOGGING_ERROR(type + " ERROR: " + infoLog);
                    return true;
                }
            } else {
                glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &isCompiled);
                if (!isCompiled) {
                    glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &maxLength);
                    char* infoLog = new char[maxLength];
                    glGetShaderInfoLog(shaderHandle, maxLength, &maxLength, infoLog);
                    CG_LOGGING_ERROR(type + " ERROR: " + infoLog);
                    return true;
                }
            }
            return false;
        }

        int getShaderUniformLocation(uint32_t programHandle, const std::string& name) {
            int32_t location = glGetUniformLocation(programHandle, name.c_str());

            #ifdef CG_ENABLE_DEBUG_FEATURES
                if (location == -1) {
                    CG_LOGGING_WARNING("Uniform doesn't exist! Uniform: {}", name)
                }
            #endif

            return location;
        }

        bool createShaderType(unsigned int type, const std::string& sType, const std::vector<char>& source, unsigned int attachTo) {
            uint32_t id = glCreateShader(type);
            glShaderBinary(1, &id, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, source.data(), static_cast<int>(source.size()) * sizeof(char));
            glSpecializeShader(id, "main", 0, nullptr, nullptr);
            bool error = OpenGLHelpers::checkShaderErrors(id, sType);
            glAttachShader(attachTo, id);
            glDeleteShader(id);
            return error;
        }

        glm::vec3 textureBorderColorToGLMVec3(TextureBorderColor color) {
            switch (color) {
                case TextureBorderColor::OpaqueBlack:
                    return glm::vec3(0.0f, 0.0f, 0.0f);
                case TextureBorderColor::OpaqueWhite:
                    return glm::vec3(1.0f, 1.0f, 1.0f);
            }
            return glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
}
