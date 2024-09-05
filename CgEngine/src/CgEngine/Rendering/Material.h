#pragma once

#include "Uuid.h"
#include "Shader.h"
#include "Texture.h"

namespace CgEngine {

    struct MaterialTextureData {
        uint32_t textureRendererId;
        uint32_t textureSlot;
    };

    class Material {
    public:
        explicit Material();

        const Uuid& getUuid() const;

        bool operator ==(const Material& other) const {
            return uuid == other.uuid;
        }

        void uploadToShader(Shader& shader) const;

    protected:
        std::unordered_map<std::string, bool> boolValues{};
        std::unordered_map<std::string, int> intValues{};
        std::unordered_map<std::string, float> floatValues{};
        std::unordered_map<std::string, glm::vec2> vec2Values{};
        std::unordered_map<std::string, glm::vec3> vec3Values{};
        std::unordered_map<std::string, glm::vec4> vec4Values{};
        std::unordered_map<std::string, glm::mat3> mat3Values{};
        std::unordered_map<std::string, glm::mat4> mat4Values{};
        std::unordered_map<std::string, MaterialTextureData> texValues{};

    private:
        Uuid uuid;
    };

}
