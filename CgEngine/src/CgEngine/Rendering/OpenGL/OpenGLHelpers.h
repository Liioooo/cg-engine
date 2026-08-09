#pragma once

#include "CgEngineSharedUtils/Enums.h"
#include "Rendering/OpenGL/OpenGLRenderer.h"

namespace CgEngine {

    namespace OpenGLHelpers {
        unsigned int shaderDataTypeToOpenGLBaseType(ShaderDataType type);
        int depthCompareOperatorToOpenGL(DepthCompareOperator op);
        int blendingEquationToOpenGL(BlendingEquation eq);
        int blendingFunctionToOpenGL(BlendingFunction fn);
        int getOpenGLTextureInternalFormat(TextureFormat format);
        int getOpenGLTextureType(TextureFormat format);
        int getOpenGLTextureFormat(TextureFormat format);
        int getOpenGLWrapMode(TextureWrap wrap);
        int getOpenGLIndexType(IndexBufferDataType type);
        unsigned int getOpenGLDrawMode(DrawMode mode);
        void applyMipMapFiltering(MipMapFiltering mipMapFiltering, unsigned int textureType);
        int attachmentTypeToOpenGLInternalFormat(AttachmentType type);
        int shaderImageAccessToOpenGL(ShaderStorageAccess access);
        int getOpenGLTextureFormatForImageBind(TextureFormat format);
        uint32_t loadOpenGLGraphicsShader(const std::string& name, ShaderEnv env);
        uint32_t loadOpenGLGraphicsShader(const std::string& vertex, const std::string& fragment, const std::string& geometry, const std::string& tcs, const std::string& tes, ShaderEnv env);
        uint32_t loadOpenGLComputeShader(const std::string& name, ShaderEnv env);
        uint32_t loadOpenGLCustomComputeShader(const std::string& name);
        bool checkShaderErrors(uint32_t shaderHandle, const std::string& type);
        int getShaderUniformLocation(uint32_t programHandle, const std::string& name);
        bool createShaderType(unsigned int type, const std::string& sType, const std::vector<uint8_t>& source, unsigned int attachTo);
        glm::vec3 textureBorderColorToGLMVec3(TextureBorderColor color);
    }

}
