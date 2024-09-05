#include "PBRMaterial.h"
#include "Application.h"
#include "FileSystem.h"

namespace CgEngine {
    PBRMaterial* PBRMaterial::createResource(const std::string& name) {
        if (name == "default-pbr-material") {
            return new PBRMaterial();
        }

        auto& resourceManager = Application::get().getResourceManager();

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

        ApplicationOptions& applicationOptions = Application::get().getApplicationOptions();

        auto* material = new PBRMaterial();

        if (!albedo.empty()) {
            uint64_t color = std::stoul(albedo.substr(1), nullptr, 16);
            float r = ((color >> 16) & 0xFF) / 255.0f;
            float g = ((color >> 8) & 0xFF) / 255.0f;
            float b = (color & 0xFF) / 255.0f;
            material->setAlbedoColor({r, g, b});
        }
        if (!metalness.empty()) {
            material->setMetalness(std::stof(metalness));
        }
        if (!roughness.empty()) {
            material->setRoughness(std::stof(roughness));
        }
        if (!emissionTexture.empty()) {
            material->setEmissionTexture(resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(emissionTexture)));
            float emissionIntensity = emission.empty() ? 1.0f : std::stof(emission);
            material->setEmission({emissionIntensity, emissionIntensity, emissionIntensity});
        } else if (!emissionColor.empty()) {
            float emissionIntensity = emission.empty() ? 1.0f : std::stof(emission);
            uint64_t color = std::stoul(emissionColor.substr(1), nullptr, 16);
            float r = ((color >> 16) & 0xFF) / 255.0f;
            float g = ((color >> 8) & 0xFF) / 255.0f;
            float b = (color & 0xFF) / 255.0f;
            material->setEmission(glm::vec3(r, g, b) * emissionIntensity);
        }
        if (!albedoTexture.empty()) {
            std::string albedoTexturePath = FileSystem::getAsGamePath(albedoTexture);

            Texture2DResourceSpecification spec{};
            spec.srgb = albedoTextureSRGB;
            spec.wrap = TextureWrap::Repeat;
            spec.mipMapFiltering = MipMapFiltering::Trilinear;
            spec.anisotropicFiltering = applicationOptions.anisotropicFiltering;

            material->setAlbedoTexture(resourceManager.getResource<Texture2D>(albedoTexturePath, spec));
        }
        if (!metalnessTexture.empty()) {
            material->setMetalnessTexture(resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(metalnessTexture)));
        }
        if (!roughnessTexture.empty()) {
            material->setRoughnessTexture(resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(roughnessTexture)));
        }
        if (!normalTexture.empty()) {
            material->setNormalTexture(resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(normalTexture)));
        }

        return material;
    }

    PBRMaterial::PBRMaterial() : Material() {
        setAlbedoColor({1.0f, 1.0f, 1.0f});
        setMetalness(0.0f);
        setRoughness(1.0f);
        setEmission({0.0f, 0.0f, 0.0f});
        setEmissionTexture(nullptr);
        setAlbedoTexture(nullptr);
        setMetalnessTexture(nullptr);
        setRoughnessTexture(nullptr);
        setNormalTexture(nullptr);

        setLoaded();
    }

    void PBRMaterial::setAlbedoColor(glm::vec3 value) {
        vec3Values["u_Mat_AlbedoColor"] = value;
    }

    void PBRMaterial::setMetalness(float value) {
        floatValues["u_Mat_Metalness"] = value;
    }

    void PBRMaterial::setRoughness(float value) {
        floatValues["u_Mat_Roughness"] = value;
    }

    void PBRMaterial::setEmission(glm::vec3 value) {
        vec3Values["u_Mat_Emission"] = value;
    }

    void PBRMaterial::setEmissionTexture(const Texture2D* texture) {
        if (texture == nullptr) {
            texValues["u_Mat_EmissionTexture"] = {Renderer::getWhiteTexture().getRendererId(), 4};
        } else {
            texValues["u_Mat_EmissionTexture"] = {texture->getRendererId(), 4};
        }
    }

    void PBRMaterial::setAlbedoTexture(const Texture2D* texture) {
        if (texture == nullptr) {
            texValues["u_Mat_AlbedoTexture"] = {Renderer::getWhiteTexture().getRendererId(), 0};
        } else {
            texValues["u_Mat_AlbedoTexture"] = {texture->getRendererId(), 0};
        }
    }

    void PBRMaterial::setMetalnessTexture(const Texture2D* texture) {
        if (texture == nullptr) {
            texValues["u_Mat_MetalnessTexture"] = {Renderer::getWhiteTexture().getRendererId(), 2};
        } else {
            texValues["u_Mat_MetalnessTexture"] = {texture->getRendererId(), 2};
        }
    }

    void PBRMaterial::setRoughnessTexture(const Texture2D* texture) {
        if (texture == nullptr) {
            texValues["u_Mat_RoughnessTexture"] = {Renderer::getWhiteTexture().getRendererId(), 3};
        } else {
            texValues["u_Mat_RoughnessTexture"] = {texture->getRendererId(), 3};
        }
    }

    void PBRMaterial::setNormalTexture(const Texture2D* texture) {
        if (texture == nullptr) {
            texValues["u_Mat_NormalTexture"] = {Renderer::getWhiteTexture().getRendererId(), 1};
            boolValues["u_Mat_UseNormals"] = false;
        } else {
            texValues["u_Mat_NormalTexture"] = {texture->getRendererId(), 1};
            boolValues["u_Mat_UseNormals"] = true;
        }
    }
}
