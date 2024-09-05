#include "Shader.h"
#include "Texture.h"
#include "FileSystem.h"
#include "Logging.h"
#include "Asserts.h"
#include "glad/glad.h"

namespace CgEngine {

    namespace ShaderUtils {
        std::string loadShaderSourceCodeWithType(const std::string& name, const std::string& type) {
            return preprocessShaderCode(ShaderUtils::loadShaderSourceCode(name + "_" + type + ".glsl", false));
        }

        std::string loadShaderSourceCode(const std::string& name, bool useCache) {
            if (!FileSystem::checkFileExists(FileSystem::getAsEnginePath("shaders/" + name))) {
                return "";
            }
            return FileSystem::readFileToString(FileSystem::getAsEnginePath("shaders/" + name));
        }

        std::string preprocessShaderCode(std::string code) {
            const auto r = std::regex("#include\\s+\"(.*)\"");

            std::vector<std::pair<std::string, std::string>> results;

            std::string current = code;

            std::smatch matches;
            while (std::regex_search(current, matches, r)) {
                results.emplace_back(matches[0], matches[1]);
                current = matches.suffix();
            }

            for (const auto& result: results) {
                std::string importCode = ShaderUtils::loadShaderSourceCode(result.second, true);
                CG_ASSERT(!importCode.empty(), "Unable to load Shader: " + result.second)
                code.replace(code.find(result.first), result.first.length(), importCode);
            }

            return code;
        }

        void checkErrors(uint32_t id, const std::string &type) {
            int isCompiled;
            int maxLength;

            if (type == "PROGRAM") {
                glGetProgramiv(id, GL_LINK_STATUS, &isCompiled);
                if (!isCompiled) {
                    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxLength);
                    char* infoLog = new char[maxLength];
                    glGetProgramInfoLog(id, maxLength, &maxLength, infoLog);
                    CG_LOGGING_ERROR(type + " ERROR: " + infoLog);
                }
            } else {
                glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);
                if (!isCompiled) {
                    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
                    char* infoLog = new char[maxLength];
                    glGetShaderInfoLog(id, maxLength, &maxLength, infoLog);
                    CG_LOGGING_ERROR(type + " ERROR: " + infoLog);
                }
            }
        }

        int32_t getUniformLocation(uint32_t programId, std::unordered_map<std::string, int32_t>& uniformLocations, const std::string& name) {
            if (uniformLocations.find(name) == uniformLocations.end()) {
                int32_t location = glGetUniformLocation(programId, name.c_str());

                CG_ASSERT(location != -1, "Uniform doesn't exist! Uniform:" + name)

                uniformLocations.insert({name, location});
                return location;
            }
            return uniformLocations.at(name);
        }

        int getSizeForShaderDataType(ShaderDataType type) {
            switch (type) {
                case ShaderDataType::Float:    return 4;
                case ShaderDataType::Float2:   return 4 * 2;
                case ShaderDataType::Float3:   return 4 * 3;
                case ShaderDataType::Float4:   return 4 * 4;
                case ShaderDataType::Mat3:     return 4 * 3 * 3;
                case ShaderDataType::Mat4:     return 4 * 4 * 4;
                case ShaderDataType::Int:      return 4;
                case ShaderDataType::Int2:     return 4 * 2;
                case ShaderDataType::Int3:     return 4 * 3;
                case ShaderDataType::Int4:     return 4 * 4;
                case ShaderDataType::Bool:     return 1;
            }
            return 0;
        }

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
    }

    Shader::Shader(std::string name) : name(std::move(name)) {
        CG_LOGGING_DEBUG("Loading Shader: {0}", this->name)

        std::string vertexSource = ShaderUtils::loadShaderSourceCodeWithType(this->name, "vertex");
        std::string fragmentSource = ShaderUtils::loadShaderSourceCodeWithType(this->name, "fragment");
        std::string geometrySource = ShaderUtils::loadShaderSourceCodeWithType(this->name, "geometry");

        programId = glCreateProgram();

        if (!vertexSource.empty()) {
            const char* cString = vertexSource.c_str();
            uint32_t id = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(id, 1, &cString, nullptr);
            glCompileShader(id);
            ShaderUtils::checkErrors(id, "VERTEX");
            glAttachShader(programId, id);
        }
        if (!fragmentSource.empty()) {
            const char* cString = fragmentSource.c_str();
            uint32_t id = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(id, 1, &cString, nullptr);
            glCompileShader(id);
            ShaderUtils::checkErrors(id, "FRAGMENT");
            glAttachShader(programId, id);
        }
        if (!geometrySource.empty()) {
            const char* cString = geometrySource.c_str();
            uint32_t id = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(id, 1, &cString, nullptr);
            glCompileShader(id);
            ShaderUtils::checkErrors(id, "GEOMETRY");
            glAttachShader(programId, id);
        }

        glLinkProgram(programId);
        ShaderUtils::checkErrors(programId, "PROGRAM");

        CG_LOGGING_DEBUG("Loaded Shader: {0}", this->name)
    }

    Shader::~Shader() {
        glDeleteProgram(programId);
    }

    void Shader::bind() {
        glUseProgram(programId);
    }

    uint32_t Shader::getProgramId() const {
        return programId;
    }

    void Shader::setBool(const std::string &name, bool value) {
        glUniform1i(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void Shader::setInt(const std::string &name, int value) {
        glUniform1i(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void Shader::setFloat(const std::string &name, float value) {
        glUniform1f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void Shader::setVec2(const std::string &name, const glm::vec2 &vec) {
        glUniform2f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y);
    }

    void Shader::setVec3(const std::string &name, const glm::vec3 &vec) {
        glUniform3f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y, vec.z);
    }

    void Shader::setVec4(const std::string &name, const glm::vec4 &vec) {
        glUniform4f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y, vec.z, vec.w);
    }

    void Shader::setMat3(const std::string &name, const glm::mat3 &mat) {
        glUniformMatrix3fv(ShaderUtils::getUniformLocation(programId, uniformLocations, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void Shader::setMat4(const std::string &name, const glm::mat4 &mat) {
        glUniformMatrix4fv(ShaderUtils::getUniformLocation(programId, uniformLocations, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void Shader::setTexture(uint32_t textureRendererId, uint32_t textureUnit) {
        glBindTextureUnit(textureUnit, textureRendererId);
    }

    ComputeShader::ComputeShader(std::string name) : name(std::move(name)) {
        std::string source = ShaderUtils::loadShaderSourceCodeWithType(this->name, "comp");

        programId = glCreateProgram();

        const char* cString = source.c_str();
        uint32_t shaderId = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shaderId, 1, &cString, nullptr);
        glCompileShader(shaderId);
        ShaderUtils::checkErrors(shaderId, "COMPUTE");
        glAttachShader(programId, shaderId);

        glLinkProgram(programId);
        ShaderUtils::checkErrors(programId, "PROGRAM");
    }

    ComputeShader::~ComputeShader() {
        glDeleteProgram(programId);
    }

    void ComputeShader::bind() {
        glUseProgram(programId);
    }

    uint32_t ComputeShader::getProgramId() const {
        return programId;
    }

    void ComputeShader::dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) {
        glDispatchCompute(groupsX, groupsY, groupsZ);
    }

    void ComputeShader::waitForMemoryBarrier() {
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    void ComputeShader::setBool(const std::string &name, bool value) {
        glUniform1i(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void ComputeShader::setInt(const std::string &name, int value) {
        glUniform1i(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void ComputeShader::setFloat(const std::string &name, float value) {
        glUniform1f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void ComputeShader::setVec2(const std::string &name, const glm::vec2 &vec) {
        glUniform2f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y);
    }

    void ComputeShader::setVec3(const std::string &name, const glm::vec3 &vec) {
        glUniform3f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y, vec.z);
    }

    void ComputeShader::setVec4(const std::string &name, const glm::vec4 &vec) {
        glUniform4f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y, vec.z, vec.w);
    }

    void ComputeShader::setMat3(const std::string &name, const glm::mat3 &mat) {
        glUniformMatrix3fv(ShaderUtils::getUniformLocation(programId, uniformLocations, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void ComputeShader::setMat4(const std::string &name, const glm::mat4 &mat) {
        glUniformMatrix4fv(ShaderUtils::getUniformLocation(programId, uniformLocations, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void ComputeShader::setTexture2D(Texture2D& texture2D, uint32_t textureUnit) {
        glBindTextureUnit(textureUnit, texture2D.getRendererId());
    }

    void ComputeShader::setTexture2D(uint32_t textureRendererId, uint32_t textureUnit) {
        glBindTextureUnit(textureUnit, textureRendererId);
    }

    void ComputeShader::setImage2D(Texture2D &texture, uint32_t textureUnit, ShaderStorageAccess storageAccess, uint32_t level) {
        glBindImageTexture(textureUnit, texture.getRendererId(), level, GL_FALSE, 0, static_cast<GLuint>(storageAccess), TextureUtils::getOpenGLTextureFormatForImageBind(texture.getFormat()));
    }

    void ComputeShader::setTextureCube(TextureCube &texture, uint32_t textureUnit) {
        glBindTextureUnit(textureUnit, texture.getRendererId());

    }

    void ComputeShader::setImageCube(TextureCube& texture, uint32_t textureUnit, ShaderStorageAccess storageAccess, uint32_t level) {
        glBindImageTexture(textureUnit, texture.getRendererId(), level, GL_TRUE, 0, static_cast<GLuint>(storageAccess), TextureUtils::getOpenGLTextureFormatForImageBind(texture.getFormat()));
    }
}
