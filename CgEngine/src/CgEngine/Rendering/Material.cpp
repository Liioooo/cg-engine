#include "Material.h"
#include "Renderer.h"
#include "FileSystem.h"
#include "Application.h"

namespace CgEngine {
    Material::Material() : uuid(Uuid()) {}

    const Uuid &Material::getUuid() const {
        return uuid;
    }

    void Material::uploadToShader(Shader& shader) const {
        for (const auto &item: boolValues) {
            shader.setBool(item.first, item.second);
        }
        for (const auto &item: intValues) {
            shader.setInt(item.first, item.second);
        }
        for (const auto &item: floatValues) {
            shader.setFloat(item.first, item.second);
        }
        for (const auto &item: vec2Values) {
            shader.setVec2(item.first, item.second);
        }
        for (const auto &item: vec3Values) {
            shader.setVec3(item.first, item.second);
        }
        for (const auto &item: vec4Values) {
            shader.setVec4(item.first, item.second);
        }
        for (const auto &item: mat3Values) {
            shader.setMat3(item.first, item.second);
        }
        for (const auto &item: mat4Values) {
            shader.setMat4(item.first, item.second);
        }
        for (const auto &item: texValues) {
            shader.setTexture(item.second.textureRendererId, item.second.textureSlot);
        }
    }
}
