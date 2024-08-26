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
        static Material* createResource(const std::string& name);

        explicit Material(std::string name);

        const std::string& getName() const;
        const Uuid& getUuid() const;

        bool operator ==(const Material& other) const {
            return uuid == other.uuid;
        }

        void set(const std::string& name, bool value);
        void set(const std::string& name, int value);
        void set(const std::string& name, float value);
        void set(const std::string& name, glm::vec2 value);
        void set(const std::string& name, glm::vec3 value);
        void set(const std::string& name, glm::vec4 value);
        void set(const std::string& name, glm::mat3 value);
        void set(const std::string& name, glm::mat4 value);
        void setTexture2D(const std::string& name, const Texture2D& texture, uint32_t textureSlot);
        void setTextureCube(const std::string& name, const TextureCube& texture, uint32_t textureSlot);
        void setTexture(const std::string& name, uint32_t textureRenderId, uint32_t textureSlot);

        bool getBool(const std::string& name) const;
        int getInt(const std::string& name) const;
        float getFloat(const std::string& name) const;
        const glm::vec2& getVec2(const std::string& name) const;
        const glm::vec3& getVec3(const std::string& name) const;
        const glm::vec4& getVec4(const std::string& name) const;
        const glm::mat3& getMat3(const std::string& name) const;
        const glm::mat4& getMat4(const std::string& name) const;
        const MaterialTextureData& getTexture(const std::string& name) const;

        void uploadToShader(Shader& shader) const;

    private:
        std::string materialName;
        Uuid uuid;

        std::unordered_map<std::string, bool> boolValues{};
        std::unordered_map<std::string, int> intValues{};
        std::unordered_map<std::string, float> floatValues{};
        std::unordered_map<std::string, glm::vec2> vec2Values{};
        std::unordered_map<std::string, glm::vec3> vec3Values{};
        std::unordered_map<std::string, glm::vec4> vec4Values{};
        std::unordered_map<std::string, glm::mat3> mat3Values{};
        std::unordered_map<std::string, glm::mat4> mat4Values{};
        std::unordered_map<std::string, MaterialTextureData> texValues{};
    };

}
