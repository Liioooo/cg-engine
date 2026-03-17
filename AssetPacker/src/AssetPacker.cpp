#include "AssetPacker.h"
#include "AssetPackerLogging.h"
#include "FilesystemUtils.h"
#include "CgEngineSharedUtils/StringUtils.h"
#include "CgEngineSharedUtils/LoaderUtils.h"
#include "AssetIDGenerator.h"

namespace AssetPacker {
    AssetPacker::AssetPacker(int argc, char** argv) {
        if (argc < 5) {
            AP_LOGGING_ERROR("Provide: --config <path to config file> --output-dir <output directory>")
            exit(1);
        }

        for (int i = 1; i < argc; i++) {
            std::string_view arg = argv[i];
            if (arg == "--config" && i + 1 < argc) {
                config.configFilePath = argv[i + 1];
                ++i;
            } else if (arg == "--output-dir" && i + 1 < argc) {
                config.outputDirectory = argv[i + 1];
                ++i;
            }
        }
    }

    void AssetPacker::run() {
        iniReader = std::make_unique<INIReader>(config.configFilePath);

        std::string materialsFileRelativePath = iniReader->Get("resources", "materials_file", "");
        if (!materialsFileRelativePath.empty()) {
            std::filesystem::path materialsFilePath = FilesystemUtils::getFilePathOfFileRelativeToConfigFile(config.configFilePath, materialsFileRelativePath);
            if (!FilesystemUtils::checkFileExists(materialsFilePath)) {
                AP_LOGGING_ERROR("Materials file does not exist: {}", materialsFilePath.string())
                exit(1);
            }

            packMaterialsFromXML(materialsFilePath);
        }

        std::string pyhsicsMaterialsFileRelativePath = iniReader->Get("resources", "physics_materials_file", "");
        if (!pyhsicsMaterialsFileRelativePath.empty()) {
            std::filesystem::path pyhsicsMaterialsFilePath = FilesystemUtils::getFilePathOfFileRelativeToConfigFile(config.configFilePath, pyhsicsMaterialsFileRelativePath);
            if (!FilesystemUtils::checkFileExists(pyhsicsMaterialsFilePath)) {
                AP_LOGGING_ERROR("pyhsicsMaterials file does not exist: {}", pyhsicsMaterialsFilePath.string())
                exit(1);
            }

            packPhysicsMaterialsFromXML(pyhsicsMaterialsFilePath);
        }
    }

    std::tuple<CgEngine::MipMapFiltering, CgEngine::TextureWrap, bool> AssetPacker::getTextureSettingsFromXMLNode(const pugi::xml_node& xmlNode) {
        CgEngine::MipMapFiltering mipMapFiltering = CgEngine::MipMapFiltering::Anisotropic;
        CgEngine::TextureWrap textureWrap = CgEngine::TextureWrap::Repeat;
        bool srgb = false;

        std::string mipMapFilteringStr = xmlNode.attribute("mipMapFiltering").as_string("");
        if (mipMapFilteringStr == "Nearest") {
            mipMapFiltering = CgEngine::MipMapFiltering::Nearest;
        } else if (mipMapFilteringStr == "Bilinear") {
            mipMapFiltering = CgEngine::MipMapFiltering::Bilinear;
        } else if (mipMapFilteringStr == "Trilinear") {
            mipMapFiltering = CgEngine::MipMapFiltering::Trilinear;
        } else if (mipMapFilteringStr == "Anisotropic") {
            mipMapFiltering = CgEngine::MipMapFiltering::Anisotropic;
        }

        std::string textureWrapStr = xmlNode.attribute("wrap").as_string("");
        if (textureWrapStr == "Clamp") {
            textureWrap = CgEngine::TextureWrap::Clamp;
        } else if (textureWrapStr == "ClampBorder") {
            textureWrap = CgEngine::TextureWrap::ClampBorder;
        } else if (textureWrapStr == "Repeat") {
            textureWrap = CgEngine::TextureWrap::Repeat;
        }

        srgb = xmlNode.attribute("srgb").as_bool(false);

        return {mipMapFiltering, textureWrap, srgb};
    }

    std::string AssetPacker::convertTextureParamsToString(const std::tuple<CgEngine::MipMapFiltering, CgEngine::TextureWrap, bool>& params) {
        std::string result;

        switch (std::get<0>(params)) {
            case CgEngine::MipMapFiltering::Nearest:
                result += "Nearest";
                break;
            case CgEngine::MipMapFiltering::Bilinear:
                result += "Bilinear";
                break;
            case CgEngine::MipMapFiltering::Trilinear:
                result += "Trilinear";
                break;
            case CgEngine::MipMapFiltering::Anisotropic:
                result += "Anisotropic";
                break;
        }

        result += "_";

        switch (std::get<1>(params)) {
            case CgEngine::TextureWrap::Clamp:
                result += "Clamp";
                break;
            case CgEngine::TextureWrap::ClampBorder:
                result += "ClampBorder";
                break;
            case CgEngine::TextureWrap::Repeat:
                result += "Repeat";
                break;
        }

        if (std::get<2>(params)) {
            result += "_sRGB";
        } else {
            result += "_Linear";
        }

        return result;
    }

    void AssetPacker::packMaterialsFromXML(const std::filesystem::path& materialsXMLPath) {
        pugi::xml_document xmlDocument;
        xmlDocument.load(FilesystemUtils::readFileToString(materialsXMLPath).c_str());

        const auto& materials = xmlDocument.child("Materials");
        for (const auto& materialNode: materials.children()) {
            std::string albedo = materialNode.child("Albedo").child_value();
            std::string metalness = materialNode.child("Metalness").child_value();
            std::string roughness = materialNode.child("Roughness").child_value();
            std::string emission = materialNode.child("Emission").child_value();
            std::string emissionColor = materialNode.child("EmissionColor").child_value();
            std::string albedoTexture = materialNode.child("AlbedoTexture").child_value();
            std::string metalnessTexture = materialNode.child("MetalnessTexture").child_value();
            std::string roughnessTexture = materialNode.child("RoughnessTexture").child_value();
            std::string emissionTexture = materialNode.child("EmissionTexture").child_value();
            std::string normalTexture = materialNode.child("NormalTexture").child_value();

            CgEngine::Assets::PackedMaterialAsset packedMaterial{};

            packedMaterial.name = materialNode.attribute("name").as_string();

            if (!albedo.empty()) {
                packedMaterial.data.albedoColor = CgEngine::LoaderUtils::hexStringToColor(albedo);
            }
            if (!metalness.empty()) {
                packedMaterial.data.metalness = CgEngine::StringUtils::toFloat(metalness).value_or(0.0f);
            }
            if (!roughness.empty()) {
                packedMaterial.data.roughness = CgEngine::StringUtils::toFloat(roughness).value_or(1.0f);
            }
            if (!emissionTexture.empty()) {
                float emissionIntensity = emission.empty() ? 1.0f : CgEngine::StringUtils::toFloat(emission).value_or(1.0f);
                packedMaterial.data.emission = {emissionIntensity, emissionIntensity, emissionIntensity};
                auto textureParams = getTextureSettingsFromXMLNode(materialNode.child("EmissionTexture"));
                packedMaterial.data.emissionTexture = packTextureFromFilePathAndParams(emissionTexture, textureParams);

            } else if (!emissionColor.empty()) {
                float emissionIntensity = emission.empty() ? 1.0f : CgEngine::StringUtils::toFloat(emission).value_or(1.0f);
                packedMaterial.data.emission = CgEngine::LoaderUtils::hexStringToColor(emissionColor) * emissionIntensity;
            }
            if (!albedoTexture.empty()) {
                auto textureParams = getTextureSettingsFromXMLNode(materialNode.child("AlbedoTexture"));
                packedMaterial.data.albedoTexture = packTextureFromFilePathAndParams(albedoTexture, textureParams);
            }
            if (!metalnessTexture.empty()) {
                auto textureParams = getTextureSettingsFromXMLNode(materialNode.child("MetalnessTexture"));
                packedMaterial.data.metalnessTexture = packTextureFromFilePathAndParams(metalnessTexture, textureParams);
            }
            if (!roughnessTexture.empty()) {
                auto textureParams = getTextureSettingsFromXMLNode(materialNode.child("RoughnessTexture"));
                packedMaterial.data.roughnessTexture = packTextureFromFilePathAndParams(roughnessTexture, textureParams);

            }
            if (!normalTexture.empty()) {
                auto textureParams = getTextureSettingsFromXMLNode(materialNode.child("NormalTexture"));
                packedMaterial.data.normalTexture = packTextureFromFilePathAndParams(normalTexture, textureParams);
            }

            AssetIDGenerator::assignAssetIdToPackedAsset(packedMaterial);
            collectedAssets.addMaterial(packedMaterial);
        }
    }

    CgEngine::Assets::AssetID AssetPacker::packTextureFromFilePathAndParams(const std::string& filePath, const std::tuple<CgEngine::MipMapFiltering, CgEngine::TextureWrap, bool>& params) {
        std::string normalizedFilePath = FilesystemUtils::normalizeFilePath(filePath);
        CgEngine::Assets::AssetID imageAssetID = packImageFromFilePath(normalizedFilePath);

        CgEngine::Assets::PackedTextureAsset packedTexture{};
        packedTexture.name = normalizedFilePath + "_" + convertTextureParamsToString(params);
        packedTexture.data.image = imageAssetID;
        packedTexture.data.mipMapFiltering = std::get<0>(params);
        packedTexture.data.wrap = std::get<1>(params);
        packedTexture.data.srgb = std::get<2>(params);

        AssetIDGenerator::assignAssetIdToPackedAsset(packedTexture);
        return collectedAssets.addTexture(packedTexture);
    }

    CgEngine::Assets::AssetID AssetPacker::packImageFromFilePath(const std::string& filePath) {
        CgEngine::Assets::AssetID alreadyPresentID = collectedAssets.getImageByName(filePath);

        if (alreadyPresentID != CgEngine::Assets::InvalidAssetID) {
            return alreadyPresentID;
        }

        CgEngine::Assets::PackedImageAsset packedImage{};
        packedImage.name = filePath;
        packedImage.data.imageData = FilesystemUtils::readFileBinary(FilesystemUtils::getFilePathOfFileRelativeToConfigFile(config.configFilePath, filePath));

        AssetIDGenerator::assignAssetIdToPackedAsset(packedImage);
        return collectedAssets.addImage(packedImage);
    }

    void AssetPacker::packPhysicsMaterialsFromXML(const std::filesystem::path& physicsMaterialsXMLPath) {
        pugi::xml_document xmlDocument;
        xmlDocument.load(FilesystemUtils::readFileToString(physicsMaterialsXMLPath).c_str());

        const auto& materials = xmlDocument.child("Materials");
        for (const auto& materialNode: materials.children()) {
            std::string staticFriction = materialNode.child("StaticFriction").child_value();
            std::string dynamicFriction = materialNode.child("DynamicFriction").child_value();
            std::string restitution = materialNode.child("Restitution").child_value();

            CgEngine::Assets::PackedPhysicsMaterialAsset packedMaterial{};

            packedMaterial.data.staticFriction = staticFriction.empty() ? 0.0f : CgEngine::StringUtils::toFloat(staticFriction).value_or(0.0f);
            packedMaterial.data.dynamicFriction = dynamicFriction.empty() ? 0.0f : CgEngine::StringUtils::toFloat(dynamicFriction).value_or(0.0f);
            packedMaterial.data.restitution = restitution.empty() ? 0.0f : CgEngine::StringUtils::toFloat(restitution).value_or(0.0f);
            packedMaterial.name = materialNode.attribute("name").as_string();

            AssetIDGenerator::assignAssetIdToPackedAsset(packedMaterial);
            collectedAssets.addPhysicsMaterial(packedMaterial);
        }
    }
}
