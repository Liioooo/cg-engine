#include <FileSystem.h>
#include <glad/glad.h>
#include <Logging.h>
#include <pugixml.hpp>
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

    CustomShader::CustomShader(std::string name, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, const std::string& tcsPath, const std::string& tesPath) : Shader() {
        CG_LOGGING_DEBUG("Loading CustomShader: {0}", this->name)

        this->name = std::move(name);

        std::string vertexSource = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(vertexPath, ShaderEnv::Custom));
        std::string fragmentSource = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(fragmentPath, ShaderEnv::Custom));
        std::string geometrySource = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(geometryPath, ShaderEnv::Custom));
        std::string tcsSource = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(tcsPath, ShaderEnv::Custom));
        std::string tesSource = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(tesPath, ShaderEnv::Custom));

        CG_ASSERT(!vertexSource.empty(), "CustomShader, Vertex Shader is required")
        CG_ASSERT(!fragmentSource.empty(), "CustomShader, Fragment Shader is required")

        programId = glCreateProgram();

        if (!vertexSource.empty()) {
            createShaderType(GL_VERTEX_SHADER, "VERTEX", vertexSource);
        }
        if (!fragmentSource.empty()) {
            createShaderType(GL_FRAGMENT_SHADER, "FRAGMENT", fragmentSource);
        }
        if (!geometrySource.empty()) {
            createShaderType(GL_GEOMETRY_SHADER, "GEOMETRY", geometrySource);
        }
        if (!tcsSource.empty()) {
            createShaderType(GL_TESS_CONTROL_SHADER, "TCS", tcsSource);
        }
        if (!tesSource.empty()) {
            createShaderType(GL_TESS_EVALUATION_SHADER, "TES", tesSource);
        }

        glLinkProgram(programId);
        ShaderUtils::checkErrors(programId, "PROGRAM");

        CG_LOGGING_DEBUG("Loaded CustomShader: {0}", this->name)
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

    CustomComputeShader::CustomComputeShader(std::string name, const std::string& path) : ComputeShader() {
        CG_LOGGING_DEBUG("Loading CustomComputeShader: {0}", this->name)

        this->name = std::move(name);

        std::string source = ShaderUtils::preprocessShaderCode(ShaderUtils::loadShaderSourceCode(path, ShaderEnv::Custom));

        CG_ASSERT(!source.empty(), "CustomComputeShader, Shader Source is required")

        programId = glCreateProgram();

        const char* cString = source.c_str();
        uint32_t shaderId = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shaderId, 1, &cString, nullptr);
        glCompileShader(shaderId);
        ShaderUtils::checkErrors(shaderId, "COMPUTE");
        glAttachShader(programId, shaderId);

        glLinkProgram(programId);
        ShaderUtils::checkErrors(programId, "PROGRAM");

        CG_LOGGING_DEBUG("Loaded CustomComputeShader: {0}", this->name)
    }
}
