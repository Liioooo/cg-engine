#include <FileSystem.h>
#include <glad/glad.h>
#include <Logging.h>
#include <pugixml.hpp>
#include <utility>
#include <Asserts.h>
#include "CustomShaders.h"

namespace CgEngine {
    const pugi::xml_document& CustomShadersData::getShadersXMLFile() {
        if (!CustomShadersData::shadersXMLFile.isLoaded()) {
            CustomShadersData::shadersXMLFile.load(FileSystem::getAsGamePath("shaders.xml"));
        }

        return CustomShadersData::shadersXMLFile.getXMLDocument();
    }

    CustomShader* CustomShader::createResource(const std::string& name) {
        const auto& shadersXML = CustomShadersData::getShadersXMLFile();

        const auto& shaders = shadersXML.child("Shaders");
        const auto& shaderNode = shaders.find_child([&name](pugi::xml_node node) {
            return std::string_view(node.name()) == "Shader" && std::string_view(node.attribute("type").as_string()) == "render" && std::string_view(node.attribute("name").as_string()) == name;
        });
        std::string vertexPath = shaderNode.child("Vertex").child_value();
        std::string fragmentPath = shaderNode.child("Fragment").child_value();
        std::string geometryPath = shaderNode.child("Geometry").child_value();
        std::string tcsPath = shaderNode.child("Tcs").child_value();
        std::string tesPath = shaderNode.child("Tes").child_value();

        return new CustomShader(name, vertexPath, fragmentPath, geometryPath, tcsPath, tesPath);
    }

    CustomShader::CustomShader(std::string name, std::string vertexPath, std::string fragmentPath, std::string geometryPath, std::string tcsPath, std::string tesPath) : Shader(), vertexPath(std::move(vertexPath)), fragmentPath(std::move(fragmentPath)), geometryPath(std::move(geometryPath)), tcsPath(std::move(tcsPath)), tesPath(std::move(tesPath)) {
        this->name = std::move(name);
        load();
    }

    void CustomShader::reload() {
        uint32_t oldId = programId;
        if (!load()) {
            clearUniformLocations();
            if (oldId != ~ 0) {
                glDeleteProgram(oldId);
            }
        }
    }

    bool CustomShader::load() {
        CG_LOGGING_DEBUG("Loading CustomShader: {0}", this->name)

        std::string vertexSource = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(vertexPath, ShaderEnv::Custom));
        std::string fragmentSource = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(fragmentPath, ShaderEnv::Custom));
        std::string geometrySource = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(geometryPath, ShaderEnv::Custom));
        std::string tcsSource = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(tcsPath, ShaderEnv::Custom));
        std::string tesSource = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(tesPath, ShaderEnv::Custom));

        CG_ASSERT(!vertexSource.empty(), "CustomShader, Vertex Shader is required")
        CG_ASSERT(!fragmentSource.empty(), "CustomShader, Fragment Shader is required")

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
            ShaderUtils::printShaderCode(vertexSource, fragmentSource, geometrySource, tcsSource, tesSource);
            glDeleteProgram(id);
        }

        CG_LOGGING_DEBUG("Loaded CustomShader: {0}", this->name)
        return error;
    }

    CustomComputeShader* CustomComputeShader::createResource(const std::string& name) {
        const auto& shadersXML = CustomShadersData::getShadersXMLFile();

        const auto& shaders = shadersXML.child("Shaders");
        const auto& shaderNode = shaders.find_child([&name](pugi::xml_node node) {
            return std::string_view(node.name()) == "Shader" && std::string_view(node.attribute("type").as_string()) == "compute" && std::string_view(node.attribute("name").as_string()) == name;
        });
        std::string path = shaderNode.child("Path").child_value();
        return new CustomComputeShader(name, path);
    }

    CustomComputeShader::CustomComputeShader(std::string name, std::string path) : ComputeShader(), path(std::move(path)) {
        this->name = std::move(name);
        load();
    }

    void CustomComputeShader::reload() {
        uint32_t oldId = programId;
        if (!load()) {
            clearUniformLocations();
            if (oldId != ~ 0) {
                glDeleteProgram(oldId);
            }
        }
    }

    bool CustomComputeShader::load() {
        CG_LOGGING_DEBUG("Loading CustomComputeShader: {0}", this->name)

        std::string source = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(path, ShaderEnv::Custom));

        CG_ASSERT(!source.empty(), "CustomComputeShader, Shader Source is required")

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

        CG_LOGGING_DEBUG("Loaded CustomComputeShader: {0}", this->name)
        return error;
    }
}
