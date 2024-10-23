#pragma once

#include "Shader.h"
#include "XMLFile.h"

namespace CgEngine {

    class CustomShader : public Shader {
    public:
        static CustomShader* createResource(const std::string& name);

        explicit CustomShader(std::string name, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, const std::string& tcsPath, const std::string& tesPath);

    private:
        static inline XMLFile shadersXMLFile;
    };

}
