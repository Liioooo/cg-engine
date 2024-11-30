#include "PBRMaterial.h"
#include "Application.h"
#include "FileSystem.h"
#include "Utils/StringUtils.h"
#include "Utils/LoaderUtils.h"

namespace CgEngine {
    PBRMaterial* PBRMaterial::createResource(const std::string& name) {
        if (name == "default-pbr-material") {
            return new PBRMaterial();
        }

       if (!xmlMaterialFile.isLoaded()) {
           xmlMaterialFile.load(FileSystem::getAsGamePath("materials.xml"));
       }

        auto& resourceManager = Application::get().getResourceManager();

        const pugi::xml_document& materialsXML = xmlMaterialFile.getXMLDocument();
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
            material->setAlbedoColor(Utils::LoaderUtils::hexStringToColor(albedo));
        }
        if (!metalness.empty()) {
            material->setMetalness(Utils::String::toFloat(metalness).value_or(0.0f));
        }
        if (!roughness.empty()) {
            material->setRoughness(Utils::String::toFloat(roughness).value_or(1.0f));
        }
        if (!emissionTexture.empty()) {
            Texture2DResourceSpecification spec{};
            spec.compression = applicationOptions.useTextureCompression;

            material->setEmissionTexture(resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(emissionTexture).string(), spec));
            float emissionIntensity = emission.empty() ? 1.0f : Utils::String::toFloat(emission).value_or(1.0f);
            material->setEmission({emissionIntensity, emissionIntensity, emissionIntensity});
        } else if (!emissionColor.empty()) {
            float emissionIntensity = emission.empty() ? 1.0f : Utils::String::toFloat(emission).value_or(1.0f);
            material->setEmission(Utils::LoaderUtils::hexStringToColor(emissionColor) * emissionIntensity);
        }
        if (!albedoTexture.empty()) {
            std::string albedoTexturePath = FileSystem::getAsGamePath(albedoTexture).string();

            Texture2DResourceSpecification spec{};
            spec.srgb = albedoTextureSRGB;
            spec.wrap = TextureWrap::Repeat;
            spec.mipMapFiltering = MipMapFiltering::Trilinear;
            spec.anisotropicFiltering = applicationOptions.anisotropicFiltering;
            spec.compression = applicationOptions.useTextureCompression;

            material->setAlbedoTexture(resourceManager.getResource<Texture2D>(albedoTexturePath, spec));
        }
        if (!metalnessTexture.empty()) {
            Texture2DResourceSpecification spec{};
            spec.compression = applicationOptions.useTextureCompression;

            material->setMetalnessTexture(resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(metalnessTexture).string(), spec));
        }
        if (!roughnessTexture.empty()) {
            Texture2DResourceSpecification spec{};
            spec.compression = applicationOptions.useTextureCompression;

            material->setRoughnessTexture(resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(roughnessTexture).string(), spec));
        }
        if (!normalTexture.empty()) {
            Texture2DResourceSpecification spec{};
            spec.compression = false;

            material->setNormalTexture(resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(normalTexture).string(), spec));
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

    void PBRMaterial::setEmissionTexture(ResRef<Texture2D> texture) {
        if (!texture) {
            texValues["u_Mat_EmissionTexture"] = {Renderer::getWhiteTexture().getRendererId(), 4};
            emissionTexture = nullptr;
        } else {
            texValues["u_Mat_EmissionTexture"] = {texture->getRendererId(), 4};
            emissionTexture = texture;
        }
    }

    void PBRMaterial::setAlbedoTexture(ResRef<Texture2D> texture) {
        if (!texture) {
            texValues["u_Mat_AlbedoTexture"] = {Renderer::getWhiteTexture().getRendererId(), 0};
            albedoTexture = nullptr;
        } else {
            texValues["u_Mat_AlbedoTexture"] = {texture->getRendererId(), 0};
            albedoTexture = texture;
        }
    }

    void PBRMaterial::setMetalnessTexture(ResRef<Texture2D> texture) {
        if (!texture) {
            texValues["u_Mat_MetalnessTexture"] = {Renderer::getWhiteTexture().getRendererId(), 2};
            metalnessTexture = nullptr;
        } else {
            texValues["u_Mat_MetalnessTexture"] = {texture->getRendererId(), 2};
            metalnessTexture = texture;
        }
    }

    void PBRMaterial::setRoughnessTexture(ResRef<Texture2D> texture) {
        if (!texture) {
            texValues["u_Mat_RoughnessTexture"] = {Renderer::getWhiteTexture().getRendererId(), 3};
            roughnessTexture = nullptr;
        } else {
            texValues["u_Mat_RoughnessTexture"] = {texture->getRendererId(), 3};
            roughnessTexture = texture;
        }
    }

    void PBRMaterial::setNormalTexture(ResRef<Texture2D> texture) {
        if (!texture) {
            texValues["u_Mat_NormalTexture"] = {Renderer::getWhiteTexture().getRendererId(), 1};
            boolValues["u_Mat_UseNormals"] = false;
            normalTexture = nullptr;
        } else {
            texValues["u_Mat_NormalTexture"] = {texture->getRendererId(), 1};
            boolValues["u_Mat_UseNormals"] = true;
            normalTexture = texture;
        }
    }
}
