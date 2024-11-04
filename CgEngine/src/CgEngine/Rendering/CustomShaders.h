#pragma once

#include "Shader.h"
#include "XMLFile.h"

namespace CgEngine {

    namespace CustomShadersData {
        static inline XMLFile shadersXMLFile;

        const pugi::xml_document& getShadersXMLFile();
    }

    class CustomShader : public Shader {
    public:
        static CustomShader* createResource(const std::string& name);

        explicit CustomShader(std::string name, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, const std::string& tcsPath, const std::string& tesPath);
    };

    class CustomComputeShader : public ComputeShader {
    public:
        static CustomComputeShader* createResource(const std::string& name);

        explicit CustomComputeShader(std::string name, const std::string& path);
    };
}
