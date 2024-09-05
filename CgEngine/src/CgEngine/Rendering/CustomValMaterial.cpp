#include "CustomValMaterial.h"

namespace CgEngine {
    void CustomValMaterial::set(const std::string& name, bool value) {
        boolValues[name] = value;
    }

    void CustomValMaterial::set(const std::string& name, int value) {
        intValues[name] = value;
    }

    void CustomValMaterial::set(const std::string &name, float value) {
        floatValues[name] = value;
    }

    void CustomValMaterial::set(const std::string& name, glm::vec2 value) {
        vec2Values[name] = value;
    }

    void CustomValMaterial::set(const std::string& name, glm::vec3 value) {
        vec3Values[name] = value;
    }

    void CustomValMaterial::set(const std::string& name, glm::vec4 value) {
        vec4Values[name] = value;
    }

    void CustomValMaterial::set(const std::string& name, glm::mat3 value) {
        mat3Values[name] = value;
    }

    void CustomValMaterial::set(const std::string& name, glm::mat4 value) {
        mat4Values[name] = value;
    }

    void CustomValMaterial::setTexture2D(const std::string& name, const Texture2D& texture, uint32_t textureSlot) {
        texValues[name] = {texture.getRendererId(), textureSlot};
    }

    void CustomValMaterial::setTextureCube(const std::string& name, const TextureCube& texture, uint32_t textureSlot) {
        texValues[name] = {texture.getRendererId(), textureSlot};
    }

    void CustomValMaterial::setTexture(const std::string& name, uint32_t textureRenderId, uint32_t textureSlot) {
        texValues[name] = {textureRenderId, textureSlot};
    }

    bool CustomValMaterial::getBool(const std::string& name) const {
        return boolValues.at(name);
    }

    int CustomValMaterial::getInt(const std::string& name) const {
        return intValues.at(name);
    }

    float CustomValMaterial::getFloat(const std::string& name) const {
        return floatValues.at(name);
    }

    const glm::vec2& CustomValMaterial::getVec2(const std::string& name) const {
        return vec2Values.at(name);
    }

    const glm::vec3& CustomValMaterial::getVec3(const std::string& name) const {
        return vec3Values.at(name);
    }

    const glm::vec4& CustomValMaterial::getVec4(const std::string& name) const {
        return vec4Values.at(name);
    }

    const glm::mat3& CustomValMaterial::getMat3(const std::string& name) const {
        return mat3Values.at(name);
    }

    const glm::mat4& CustomValMaterial::getMat4(const std::string& name) const {
        return mat4Values.at(name);
    }

    const MaterialTextureData& CustomValMaterial::getTexture(const std::string& name) const {
        return texValues.at(name);
    }
}
