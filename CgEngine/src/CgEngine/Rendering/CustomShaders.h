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

        explicit CustomShader(std::string name, std::string vertexPath, std::string fragmentPath, std::string geometryPath, std::string tcsPath, std::string tesPath);

        CustomShader(CustomShader& other) = delete;
        CustomShader& operator=(CustomShader& other) = delete;

        void reload() override;

    private:
        bool load();

        std::string vertexPath;
        std::string fragmentPath;
        std::string geometryPath;
        std::string tcsPath;
        std::string tesPath;
    };

    class CustomComputeShader : public ComputeShader {
    public:
        static CustomComputeShader* createResource(const std::string& name);

        explicit CustomComputeShader(std::string name, std::string path);

        CustomComputeShader(CustomComputeShader& other) = delete;
        CustomComputeShader& operator=(CustomComputeShader& other) = delete;

        void reload() override;

    private:
        bool load();

        std::string path;
    };
}
