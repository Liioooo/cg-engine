#pragma once

#include "MemoryBarrierBit.h"

namespace CgEngine {

    class Texture2D;
    class TextureCube;
    class Texture2DArray;

    enum class ShaderStorageAccess {
        WriteOnly = 0x88B9, // GL_WRITE_ONLY
        ReadOnly = 0x88B8, // GL_READ_ONLY
        ReadWrite = 0x88BA, // GL_READ_WRITE
    };

    enum class ShaderDataType {
        Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
    };

    enum class ShaderEnv {
        Engine, Custom
    };

    namespace ShaderUtils {
        std::string loadShaderSourceCodeWithType(const std::string& name, const std::string& type, ShaderEnv env);
        std::string loadShaderSourceCode(const std::string& name, ShaderEnv env);
        std::string preprocessShaderCode(std::string code, const std::vector<std::filesystem::path>& alreadyImported = {});
        bool checkErrors(uint32_t id, const std::string &type);
        int32_t getUniformLocation(uint32_t programId, std::unordered_map<std::string, int32_t>& uniformLocations, const std::string& name);

        int getSizeForShaderDataType(ShaderDataType type);
        unsigned int shaderDataTypeToOpenGLBaseType(ShaderDataType type);

        std::string getSourceWithLineNumbers(const std::string& source);
        void printShaderCode(std::string vertexSource, std::string fragmentSource, std::string geometrySource, std::string tcsSource, std::string tesSource);
    }

    class Shader {
    public:
        Shader() = default;
        explicit Shader(std::string name);
        virtual ~Shader();

        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        Shader(Shader& other) = delete;
        Shader& operator=(Shader& other) = delete;

        virtual void reload();

        void bind();
        bool isReady() const;

        uint32_t getProgramId() const;
        std::string getName() const;

        void setBool(const std::string& name, bool value);
        void setInt(const std::string& name, int value);
        void setFloat(const std::string& name, float value);
        void setVec2(const std::string& name, const glm::vec2& vec);
        void setVec3(const std::string& name, const glm::vec3& vec);
        void setVec4(const std::string& name, const glm::vec4& vec);
        void setMat3(const std::string& name, const glm::mat3& mat);
        void setMat4(const std::string& name, const glm::mat4& mat);
        void setTexture(uint32_t textureRendererId, uint32_t textureUnit);

    protected:
        bool createShaderType(unsigned int type, const std::string& sType, const std::string& source, unsigned int attachTo);
        void clearUniformLocations();

        std::string name;
        uint32_t programId = ~0;

    private:
        std::unordered_map<std::string, int32_t> uniformLocations{};

        bool load();
    };

    class ComputeShader {
    public:
        ComputeShader() = default;
        explicit ComputeShader(std::string name);
        ~ComputeShader();

        ComputeShader(ComputeShader&& other) noexcept;
        ComputeShader& operator= (ComputeShader&& other) noexcept;

        ComputeShader(ComputeShader& other) = delete;
        ComputeShader& operator=(ComputeShader& other) = delete;

        virtual void reload();

        void bind();
        bool isReady() const;

        uint32_t getProgramId() const;
        std::string getName() const;

        void dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ);
        void waitForMemoryBarrier(std::initializer_list<MemoryBarrierBit> barriers);

        void setBool(const std::string& name, bool value);
        void setInt(const std::string& name, int value);
        void setFloat(const std::string& name, float value);
        void setVec2(const std::string& name, const glm::vec2& vec);
        void setVec3(const std::string& name, const glm::vec3& vec);
        void setVec4(const std::string& name, const glm::vec4& vec);
        void setMat3(const std::string& name, const glm::mat3& mat);
        void setMat4(const std::string& name, const glm::mat4& mat);
        void setTexture2D(Texture2D& texture, uint32_t textureUnit);
        void setTexture2D(uint32_t textureRendererId, uint32_t textureUnit);
        void setImage2D(Texture2D& texture, uint32_t textureUnit, ShaderStorageAccess storageAccess, uint32_t level = 0);
        void setTextureCube(TextureCube& texture, uint32_t textureUnit);
        void setImageCube(TextureCube& texture, uint32_t textureUnit, ShaderStorageAccess storageAccess, uint32_t level = 0);
        void setImageArray(Texture2DArray& texture, uint32_t textureUnit, ShaderStorageAccess storageAccess);

    protected:
        void clearUniformLocations();

        std::string name;
        uint32_t programId = ~0;

    private:
        std::unordered_map<std::string, int32_t> uniformLocations{};

        bool load();
    };

}
