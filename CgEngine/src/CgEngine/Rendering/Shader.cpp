#include "Shader.h"
#include "Texture.h"
#include "FileSystem.h"
#include "Logging.h"
#include "Asserts.h"
#include "glad/glad.h"

namespace CgEngine {

    namespace ShaderUtils {
        std::string loadShaderSourceCodeWithType(const std::string& name, const std::string& type, ShaderEnv env) {
            return preprocessShaderCode(ShaderUtils::loadShaderSourceCode(name + "_" + type + ".glsl", env));
        }

        std::string loadShaderSourceCode(const std::string& name, ShaderEnv env) {
            const std::filesystem::path path = env == ShaderEnv::Engine ? FileSystem::getAsEnginePath(std::filesystem::path("./shaders/") / name) : FileSystem::getAsGamePath(name);

            if (!FileSystem::checkFileExists(path)) {
                return "";
            }
            return FileSystem::readFileToString(path);
        }

        std::string preprocessShaderCode(std::string code, const std::vector<std::filesystem::path>& alreadyImported) {
            std::array<std::filesystem::path, 2> includeDirs = {
                    FileSystem::getAsGamePath("./"),
                    FileSystem::getAsEnginePath("./shaders/")
            };

            const auto r = std::regex("#include\\s+\"(.*)\"");

            std::vector<std::pair<std::string, std::string>> results;

            std::string current = code;

            std::smatch matches;
            while (std::regex_search(current, matches, r)) {
                results.emplace_back(matches[0], matches[1]);
                current = matches.suffix();
            }

            for (const auto& result: results) {
                for (const auto& dir: includeDirs) {
                    const std::filesystem::path path = dir / result.second;

                    if (std::find(alreadyImported.begin(), alreadyImported.end(), path) != alreadyImported.end()) {
                        code.replace(code.find(result.first), result.first.length(), "");
                        break;
                    }

                    if (FileSystem::checkFileExists(path)) {
                        const_cast<std::vector<std::filesystem::path>&>(alreadyImported).emplace_back(path);
                        std::string importCode = preprocessShaderCode(FileSystem::readFileToString(path), alreadyImported);
                        CG_ASSERT(!importCode.empty(), "Unable to load Shader: " + result.second)
                        code.replace(code.find(result.first), result.first.length(), importCode);
                        break;
                    }
                }
            }

            return code;
        }

        bool checkErrors(uint32_t id, const std::string &type) {
            int isCompiled;
            int maxLength;

            if (type == "PROGRAM") {
                glGetProgramiv(id, GL_LINK_STATUS, &isCompiled);
                if (!isCompiled) {
                    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxLength);
                    char* infoLog = new char[maxLength];
                    glGetProgramInfoLog(id, maxLength, &maxLength, infoLog);
                    CG_LOGGING_ERROR(type + " ERROR: " + infoLog);
                    return true;
                }
            } else {
                glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);
                if (!isCompiled) {
                    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
                    char* infoLog = new char[maxLength];
                    glGetShaderInfoLog(id, maxLength, &maxLength, infoLog);
                    CG_LOGGING_ERROR(type + " ERROR: " + infoLog);
                    return true;
                }
            }
            return false;
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
        load();
    }

    Shader::~Shader() {
        if (programId != ~0) {
            glDeleteProgram(programId);
        }
    }

    Shader::Shader(Shader&& other) noexcept {
        programId = other.programId;
        name = std::move(other.name);
        uniformLocations = std::move(other.uniformLocations);

        other.programId = ~0;
    }

    Shader& Shader::operator=(Shader&& other) noexcept {
        if (this != &other) {
            if (programId != ~0) {
                glDeleteProgram(programId);
            }

            programId = other.programId;
            name = std::move(other.name);
            uniformLocations = std::move(other.uniformLocations);

            other.programId = ~0;
        }
        return *this;
    }

    void Shader::reload() {
        uint32_t oldId = programId;
        if (!load()) {
            clearUniformLocations();
            if (oldId != ~ 0) {
                glDeleteProgram(oldId);
            }
        }
    }

    void Shader::bind() {
        CG_ASSERT(isReady(), "Shader is not ready!")
        glUseProgram(programId);
    }

    bool Shader::isReady() const {
        return programId != ~0;
    }

    uint32_t Shader::getProgramId() const {
        return programId;
    }

    std::string Shader::getName() const {
        return name;
    }

    void Shader::setBool(const std::string &name, bool value) {
        CG_ASSERT(isReady(), "Shader is not ready!")
        glUniform1i(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void Shader::setInt(const std::string &name, int value) {
        CG_ASSERT(isReady(), "Shader is not ready!")
        glUniform1i(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void Shader::setFloat(const std::string &name, float value) {
        CG_ASSERT(isReady(), "Shader is not ready!")
        glUniform1f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void Shader::setVec2(const std::string &name, const glm::vec2 &vec) {
        CG_ASSERT(isReady(), "Shader is not ready!")
        glUniform2f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y);
    }

    void Shader::setVec3(const std::string &name, const glm::vec3 &vec) {
        CG_ASSERT(isReady(), "Shader is not ready!")
        glUniform3f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y, vec.z);
    }

    void Shader::setVec4(const std::string &name, const glm::vec4 &vec) {
        CG_ASSERT(isReady(), "Shader is not ready!")
        glUniform4f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y, vec.z, vec.w);
    }

    void Shader::setMat3(const std::string &name, const glm::mat3 &mat) {
        CG_ASSERT(isReady(), "Shader is not ready!")
        glUniformMatrix3fv(ShaderUtils::getUniformLocation(programId, uniformLocations, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void Shader::setMat4(const std::string &name, const glm::mat4 &mat) {
        CG_ASSERT(isReady(), "Shader is not ready!")
        glUniformMatrix4fv(ShaderUtils::getUniformLocation(programId, uniformLocations, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void Shader::setTexture(uint32_t textureRendererId, uint32_t textureUnit) {
        CG_ASSERT(isReady(), "Shader is not ready!")
        glBindTextureUnit(textureUnit, textureRendererId);
    }

    bool Shader::createShaderType(unsigned int type, const std::string& sType, const std::string& source, unsigned int attachTo) {
        const char* cString = source.c_str();
        uint32_t id = glCreateShader(type);
        glShaderSource(id, 1, &cString, nullptr);
        glCompileShader(id);
        bool error = ShaderUtils::checkErrors(id, sType);
        glAttachShader(attachTo, id);
        glDeleteShader(id);
        return error;
    }

    void Shader::clearUniformLocations() {
        uniformLocations.clear();
    }

    bool Shader::load() {
        CG_LOGGING_DEBUG("Loading Shader: {0}", this->name)

        std::string vertexSource = ShaderUtils::loadShaderSourceCodeWithType(this->name, "vertex", ShaderEnv::Engine);
        std::string fragmentSource = ShaderUtils::loadShaderSourceCodeWithType(this->name, "fragment", ShaderEnv::Engine);
        std::string geometrySource = ShaderUtils::loadShaderSourceCodeWithType(this->name, "geometry", ShaderEnv::Engine);
        std::string tcsSource = ShaderUtils::loadShaderSourceCodeWithType(this->name, "tcs", ShaderEnv::Engine);
        std::string tesSource = ShaderUtils::loadShaderSourceCodeWithType(this->name, "tes", ShaderEnv::Engine);

        uint32_t id = glCreateProgram();
        bool error = false;

        if (!vertexSource.empty()) {
            error |= createShaderType(GL_VERTEX_SHADER, "VERTEX", vertexSource, id);
        }
        if (!fragmentSource.empty()) {
            error |= createShaderType(GL_FRAGMENT_SHADER, "FRAGMENT", fragmentSource, id);
        }
        if (!geometrySource.empty()) {
            error |= createShaderType(GL_GEOMETRY_SHADER, "GEOMETRY", geometrySource, id);
        }
        if (!tcsSource.empty()) {
            error |= createShaderType(GL_TESS_CONTROL_SHADER, "TCS", tcsSource, id);
        }
        if (!tesSource.empty()) {
            error |= createShaderType(GL_TESS_EVALUATION_SHADER, "TES", tesSource, id);
        }

        glLinkProgram(id);
        error |= ShaderUtils::checkErrors(id, "PROGRAM");

        if (!error) {
            programId = id;
        } else {
            glDeleteProgram(id);
        }

        CG_LOGGING_DEBUG("Loaded Shader: {0}", this->name)
        return error;
    }

    ComputeShader::ComputeShader(std::string name) : name(std::move(name)) {
        load();
    }

    ComputeShader::~ComputeShader() {
        if (programId != ~0) {
            glDeleteProgram(programId);
        }
    }

    ComputeShader::ComputeShader(ComputeShader&& other) noexcept {
        programId = other.programId;
        name = std::move(name);
        uniformLocations = std::move(other.uniformLocations);

        other.programId = ~0;
    }

    ComputeShader& ComputeShader::operator=(ComputeShader&& other) noexcept {
        if (this != &other) {
            if (programId != ~0) {
                glDeleteProgram(programId);
            }

            programId = other.programId;
            name = std::move(name);
            uniformLocations = std::move(other.uniformLocations);

            other.programId = ~0;
        }
        return *this;
    }

    void ComputeShader::reload() {
        uint32_t oldId = programId;
        if (!load()) {
            clearUniformLocations();
            if (oldId != ~ 0) {
                glDeleteProgram(oldId);
            }
        }
    }

    void ComputeShader::bind() {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glUseProgram(programId);
    }

    bool ComputeShader::isReady() const {
        return programId != ~0;
    }

    uint32_t ComputeShader::getProgramId() const {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        return programId;
    }

    std::string ComputeShader::getName() const {
        return name;
    }

    void ComputeShader::dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glDispatchCompute(groupsX, groupsY, groupsZ);
    }

    void ComputeShader::waitForMemoryBarrier(std::initializer_list<MemoryBarrierBit> barriers) {
        glMemoryBarrier(MemoryBarrierUtils::convertToBitfield(barriers));
    }

    void ComputeShader::setBool(const std::string &name, bool value) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glUniform1i(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void ComputeShader::setInt(const std::string &name, int value) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glUniform1i(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void ComputeShader::setFloat(const std::string &name, float value) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glUniform1f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), value);
    }

    void ComputeShader::setVec2(const std::string &name, const glm::vec2 &vec) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glUniform2f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y);
    }

    void ComputeShader::setVec3(const std::string &name, const glm::vec3 &vec) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glUniform3f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y, vec.z);
    }

    void ComputeShader::setVec4(const std::string &name, const glm::vec4 &vec) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glUniform4f(ShaderUtils::getUniformLocation(programId, uniformLocations, name), vec.x, vec.y, vec.z, vec.w);
    }

    void ComputeShader::setMat3(const std::string &name, const glm::mat3 &mat) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glUniformMatrix3fv(ShaderUtils::getUniformLocation(programId, uniformLocations, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void ComputeShader::setMat4(const std::string &name, const glm::mat4 &mat) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glUniformMatrix4fv(ShaderUtils::getUniformLocation(programId, uniformLocations, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void ComputeShader::setTexture2D(Texture2D& texture2D, uint32_t textureUnit) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glBindTextureUnit(textureUnit, texture2D.getRendererId());
    }

    void ComputeShader::setTexture2D(uint32_t textureRendererId, uint32_t textureUnit) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glBindTextureUnit(textureUnit, textureRendererId);
    }

    void ComputeShader::setImage2D(Texture2D &texture, uint32_t textureUnit, ShaderStorageAccess storageAccess, uint32_t level) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glBindImageTexture(textureUnit, texture.getRendererId(), level, GL_FALSE, 0, static_cast<GLuint>(storageAccess), TextureUtils::getOpenGLTextureFormatForImageBind(texture.getFormat()));
    }

    void ComputeShader::setTextureCube(TextureCube &texture, uint32_t textureUnit) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glBindTextureUnit(textureUnit, texture.getRendererId());
    }

    void ComputeShader::setImageCube(TextureCube& texture, uint32_t textureUnit, ShaderStorageAccess storageAccess, uint32_t level) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glBindImageTexture(textureUnit, texture.getRendererId(), level, GL_TRUE, 0, static_cast<GLuint>(storageAccess), TextureUtils::getOpenGLTextureFormatForImageBind(texture.getFormat()));
    }

    void ComputeShader::setImageArray(CgEngine::Texture2DArray& texture, uint32_t textureUnit, CgEngine::ShaderStorageAccess storageAccess) {
        CG_ASSERT(isReady(), "ComputeShader is not ready!")
        glBindImageTexture(textureUnit, texture.getRendererId(), 0, GL_TRUE, 0, static_cast<GLuint>(storageAccess), TextureUtils::getOpenGLTextureFormatForImageBind(texture.getFormat()));
    }

    void ComputeShader::clearUniformLocations() {
        uniformLocations.clear();
    }

    bool ComputeShader::load() {
        CG_LOGGING_DEBUG("Loading ComputeShader: {0}", this->name)

        std::string source = ShaderUtils::loadShaderSourceCodeWithType(this->name, "comp", ShaderEnv::Engine);

        uint32_t id = glCreateProgram();
        bool error = false;

        const char* cString = source.c_str();
        uint32_t shaderId = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shaderId, 1, &cString, nullptr);
        glCompileShader(shaderId);
        error |= ShaderUtils::checkErrors(shaderId, "COMPUTE");
        glAttachShader(id, shaderId);
        glDeleteShader(shaderId);

        glLinkProgram(id);
        error |= ShaderUtils::checkErrors(id, "PROGRAM");

        if (!error) {
            programId = id;
        } else {
            glDeleteProgram(id);
        }

        CG_LOGGING_DEBUG("Loaded ComputeShader: {0}", this->name)
        return error;
    }
}
