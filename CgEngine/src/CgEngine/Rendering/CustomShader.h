#pragma once

#include "Resources/Resource.h"
#include "Shader.h"
#include "XMLFile.h"

namespace CgEngine {

    class CustomShader : public Shader, public Resource {
    public:
        static CustomShader* createResource(const std::string& name);

        explicit CustomShader(std::string name, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath);

    private:
        static inline XMLFile shadersXMLFile;
    };

}
