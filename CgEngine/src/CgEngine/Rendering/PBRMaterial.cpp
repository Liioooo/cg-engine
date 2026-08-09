#include "PBRMaterial.h"
#include "Application.h"
#include "FileSystem.h"
#include "CgEngineSharedUtils/StringUtils.h"
#include "CgEngineSharedUtils/LoaderUtils.h"
#include "GraphicsObjectsFactory.h"

namespace CgEngine {
    PBRMaterial* PBRMaterial::createResource(const std::string& name) {
        if (name == "default-pbr-material") {
            return new PBRMaterial(PBRMaterialSpecification());
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

        PBRMaterialSpecification materialSpecification;

        if (!albedo.empty()) {
            materialSpecification.albedoColor = LoaderUtils::hexStringToColor(albedo);
        }
        if (!metalness.empty()) {
            materialSpecification.metalness = StringUtils::toFloat(metalness).value_or(0.0f);
        }
        if (!roughness.empty()) {
            materialSpecification.roughness = StringUtils::toFloat(roughness).value_or(1.0f);
        }
        if (!emissionTexture.empty()) {
            materialSpecification.emissionTexture = resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(emissionTexture).string());
            float emissionIntensity = emission.empty() ? 1.0f : StringUtils::toFloat(emission).value_or(1.0f);
            materialSpecification.emission = {emissionIntensity, emissionIntensity, emissionIntensity};
        } else if (!emissionColor.empty()) {
            float emissionIntensity = emission.empty() ? 1.0f : StringUtils::toFloat(emission).value_or(1.0f);
            materialSpecification.emission = LoaderUtils::hexStringToColor(emissionColor) * emissionIntensity;
        }
        if (!albedoTexture.empty()) {
            std::string albedoTexturePath = FileSystem::getAsGamePath(albedoTexture).string();

            Texture2DResourceSpecification spec{};
            spec.srgb = albedoTextureSRGB;
            spec.wrap = TextureWrap::Repeat;
            spec.mipMapFiltering = MipMapFiltering::Anisotropic;

            materialSpecification.albedoTexture = resourceManager.getResource<Texture2D>(albedoTexturePath, spec);
        }
        if (!metalnessTexture.empty()) {
            Texture2DResourceSpecification spec{};

            materialSpecification.metalnessTexture = resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(metalnessTexture).string(), spec);
        }
        if (!roughnessTexture.empty()) {
            Texture2DResourceSpecification spec{};

           materialSpecification.roughnessTexture = resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(roughnessTexture).string(), spec);
        }
        if (!normalTexture.empty()) {
            materialSpecification.normalTexture = resourceManager.getResource<Texture2D>(FileSystem::getAsGamePath(normalTexture).string());
        }

        return new PBRMaterial(materialSpecification);
    }

    PBRMaterial::PBRMaterial(PBRMaterialSpecification spec) {
        materialBuffer = GraphicsObjectsFactory::createUniformBuffer(sizeof(PBRMaterialData));

        PBRMaterialData materialData{};
        materialData.roughness = spec.roughness;
        materialData.metalness = spec.metalness;
        materialData.albedoColor = spec.albedoColor;
        materialData.emission = spec.emission;
        materialData.useNormals = spec.normalTexture != nullptr ? 1 : 0;

        materialBuffer->setData(&materialData, sizeof(PBRMaterialData));

        DescriptorSetSpecification descriptorSetSpec{};
        descriptorSetSpec.layout = Application::get().getSceneRenderer().getPBRMaterialDescriptorSetLayout();
        descriptorSetSpec.texture2DBindings = {
            {0, spec.albedoTexture ? spec.albedoTexture.get() : Renderer::getWhiteTexture()},
            {1, spec.normalTexture ? spec.normalTexture.get() : Renderer::getWhiteTexture()},
            {2, spec.metalnessTexture ? spec.metalnessTexture.get() : Renderer::getWhiteTexture()},
            {3, spec.roughnessTexture ? spec.roughnessTexture.get() : Renderer::getWhiteTexture()},
            {4, spec.emissionTexture ? spec.emissionTexture.get() : Renderer::getWhiteTexture()}
        };
        descriptorSetSpec.uboBindings = {
            {5, materialBuffer}
        };

        descriptorSet = GraphicsObjectsFactory::createDescriptorSet(descriptorSetSpec);

        if (spec.albedoTexture) {
           albedoTexture = spec.albedoTexture;
        } else {
            albedoTexture = nullptr;
        }

        if (spec.normalTexture) {
            normalTexture = spec.normalTexture;
        } else {
            normalTexture = nullptr;
        }

        if (spec.metalnessTexture) {
            metalnessTexture = spec.metalnessTexture;
        } else {
            metalnessTexture = nullptr;
        }

        if (spec.roughnessTexture) {
            roughnessTexture = spec.roughnessTexture;
        } else {
            roughnessTexture = nullptr;
        }

        if (spec.emissionTexture) {
            emissionTexture = spec.emissionTexture;
        } else {
            emissionTexture = nullptr;
        }
    }

    PBRMaterial::~PBRMaterial() {
        delete descriptorSet;
        delete materialBuffer;
    }

    const Uuid& PBRMaterial::getUuid() const {
        return uuid;
    }

    bool PBRMaterial::operator==(const PBRMaterial& other) const {
        return uuid == other.uuid;
    }

    DescriptorSet* PBRMaterial::getDescriptorSet() const {
        return descriptorSet;
    }
}
