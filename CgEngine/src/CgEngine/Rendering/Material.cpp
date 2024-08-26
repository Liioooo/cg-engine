#include "Material.h"
#include "Renderer.h"
#include "GlobalObjectManager.h"
#include "FileSystem.h"
#include "Application.h"

namespace CgEngine {
    Material *Material::createResource(const std::string& name) {
        auto& resourceManager = GlobalObjectManager::getInstance().getResourceManager();

        const pugi::xml_document& materialsXML = resourceManager.getResource<XMLFile>(FileSystem::getAsGamePath("materials.xml"))->getXMLDocument();
        const auto& materials = materialsXML.child("Materials");
        const auto& materialNode = materials.find_child_by_attribute("Material", "name", name.c_str());
        std::string albedo = materialNode.child("Albedo").child_value();
        std::string metalness = materialNode.child("Metalness").child_value();
        std::string roughness = materialNode.child("Roughness").child_value();
        std::string emission = materialNode.child("Emission").child_value();
        std::string emissionColor = materialNode.child("EmissionColor").child_value();
        std::string albedoTexture = materialNode.child("AlbedoTexture").child_value();
        bool albedoTextureSRGB = materialNode.child("AlbedoTexture").attribute("srgb").as_bool(false);
        std::string metalnessTexture = materialNode.child("metalnessTexture").child_value();
        std::string roughnessTexture = materialNode.child("RoughnessTexture").child_value();
        std::string emissionTexture = materialNode.child("EmissionTexture").child_value();
        std::string normalTexture = materialNode.child("NormalTexture").child_value();
        bool useNormals = strcmp(materials.child("UserNormals").child_value(), "true") == 0;

        ApplicationOptions& applicationOptions = Application::get().getApplicationOptions();

        auto* material = new Material(name);

        if (albedo.empty()) {
            material->set("u_Mat_AlbedoColor", {1.0f, 1.0f, 1.0f});
        } else {
            uint64_t color = std::stoul(albedo.substr(1), nullptr, 16);
            float r = ((color >> 16) & 0xFF) / 255.0f;
            float g = ((color >> 8) & 0xFF) / 255.0f;
            float b = (color & 0xFF) / 255.0f;
            material->set("u_Mat_AlbedoColor", {r, g, b});
        }
        if (metalness.empty()) {
            material->set("u_Mat_Metalness", 0.0f);
        } else {
            material->set("u_Mat_Metalness", std::stof(metalness));
        }
        if (roughness.empty()) {
            material->set("u_Mat_Roughness", 1.0f);
        } else {
            material->set("u_Mat_Roughness", std::stof(roughness));
        }
        if (!emissionTexture.empty()) {
            material->setTexture2D("u_Mat_EmissionTexture", *resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(emissionTexture)), 4);
            float emissionIntensity = emission.empty() ? 1.0f : std::stof(emission);
            material->set("u_Mat_Emission", {emissionIntensity, emissionIntensity, emissionIntensity});
        } else if (!emissionColor.empty()) {
            material->setTexture2D("u_Mat_EmissionTexture", Renderer::getWhiteTexture(), 4);
            float emissionIntensity = emission.empty() ? 1.0f : std::stof(emission);
            uint64_t color = std::stoul(emissionColor.substr(1), nullptr, 16);
            float r = ((color >> 16) & 0xFF) / 255.0f;
            float g = ((color >> 8) & 0xFF) / 255.0f;
            float b = (color & 0xFF) / 255.0f;
            material->set("u_Mat_Emission", glm::vec3(r, g, b) * emissionIntensity);
        } else {
            material->setTexture2D("u_Mat_EmissionTexture", Renderer::getWhiteTexture(), 4);
            material->set("u_Mat_Emission", {0.0f, 0.0f, 0.0f});
        }
        if (albedoTexture.empty()) {
            material->setTexture2D("u_Mat_AlbedoTexture", Renderer::getWhiteTexture(), 0);
        } else {
            std::string albedoTexturePath = FileSystem::getAsGamePath(albedoTexture);

            if (resourceManager.hasResource<Texture2D>(albedoTexturePath)) {
                material->setTexture2D("u_Mat_AlbedoTexture", *resourceManager.getResource<Texture2D>(albedoTexturePath), 0);
            } else {
                auto* texture = new Texture2D(albedoTexturePath, albedoTextureSRGB, TextureWrap::Repeat, MipMapFiltering::Trilinear, applicationOptions.anisotropicFiltering);
                resourceManager.insertResource(albedoTexturePath, texture);
                material->setTexture2D("u_Mat_AlbedoTexture", *texture, 0);
            }
        }
        if (metalnessTexture.empty()) {
            material->setTexture2D("u_Mat_MetalnessTexture", Renderer::getWhiteTexture(), 2);
        } else {
            material->setTexture2D("u_Mat_MetalnessTexture", *resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(metalnessTexture)), 2);
        }
        if (roughnessTexture.empty()) {
            material->setTexture2D("u_Mat_RoughnessTexture", Renderer::getWhiteTexture(), 3);
        } else {
            material->setTexture2D("u_Mat_RoughnessTexture", *resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(roughnessTexture)), 3);
        }
        if (normalTexture.empty()) {
            material->setTexture2D("u_Mat_NormalTexture", Renderer::getWhiteTexture(), 1);
            material->set("u_Mat_UseNormals", false);
        } else {
            material->setTexture2D("u_Mat_NormalTexture", *resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(normalTexture)), 1);
            material->set("u_Mat_UseNormals", useNormals);
        }

        return material;
    }

    Material::Material(std::string name) : materialName(std::move(name)), uuid(Uuid()) {}

    const std::string &Material::getName() const {
        return materialName;
    }

    const Uuid &Material::getUuid() const {
        return uuid;
    }

    void Material::set(const std::string& name, bool value) {
        boolValues[name] = value;
    }

    void Material::set(const std::string& name, int value) {
        intValues[name] = value;
    }

    void Material::set(const std::string &name, float value) {
        floatValues[name] = value;
    }

    void Material::set(const std::string& name, glm::vec2 value) {
        vec2Values[name] = value;
    }

    void Material::set(const std::string& name, glm::vec3 value) {
        vec3Values[name] = value;
    }

    void Material::set(const std::string& name, glm::vec4 value) {
        vec4Values[name] = value;
    }

    void Material::set(const std::string& name, glm::mat3 value) {
        mat3Values[name] = value;
    }

    void Material::set(const std::string& name, glm::mat4 value) {
        mat4Values[name] = value;
    }

    void Material::setTexture2D(const std::string& name, const Texture2D& texture, uint32_t textureSlot) {
        texValues[name] = {texture.getRendererId(), textureSlot};
    }

    void Material::setTextureCube(const std::string& name, const TextureCube& texture, uint32_t textureSlot) {
        texValues[name] = {texture.getRendererId(), textureSlot};
    }

    void Material::setTexture(const std::string& name, uint32_t textureRenderId, uint32_t textureSlot) {
        texValues[name] = {textureRenderId, textureSlot};
    }

    bool Material::getBool(const std::string& name) const {
        return boolValues.at(name);
    }

    int Material::getInt(const std::string& name) const {
        return intValues.at(name);
    }

    float Material::getFloat(const std::string& name) const {
        return floatValues.at(name);
    }

    const glm::vec2 &Material::getVec2(const std::string& name) const {
        return vec2Values.at(name);
    }

    const glm::vec3 &Material::getVec3(const std::string& name) const {
        return vec3Values.at(name);
    }

    const glm::vec4 &Material::getVec4(const std::string& name) const {
        return vec4Values.at(name);
    }

    const glm::mat3 &Material::getMat3(const std::string& name) const {
        return mat3Values.at(name);
    }

    const glm::mat4 &Material::getMat4(const std::string& name) const {
        return mat4Values.at(name);
    }

    const MaterialTextureData &Material::getTexture(const std::string& name) const {
        return texValues.at(name);
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
