#pragma once

#include "Material.h"

namespace CgEngine {

    class CustomValMaterial : public Material {
    public:
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

    };

}
