#include <FileSystem.h>
#include <glad/glad.h>
#include <Logging.h>
#include <pugixml.hpp>
#include <Asserts.h>
#include "CustomShader.h"

namespace CgEngine {
    CustomShader* CustomShader::createResource(const std::string& name) {
        if (!shadersXMLFile.isLoaded()) {
            shadersXMLFile.load(FileSystem::getAsGamePath("shaders.xml"));
        }

        const pugi::xml_document& shadersXML = shadersXMLFile.getXMLDocument();
        const auto& shaders = shadersXML.child("Shaders");
        const auto& shaderNode = shaders.find_child_by_attribute("Shader", "name", name.c_str());
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
}
