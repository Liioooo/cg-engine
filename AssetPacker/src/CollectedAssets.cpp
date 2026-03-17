#pragma once

#include "CollectedAssets.h"

namespace AssetPacker {

    CgEngine::Assets::AssetID CollectedAssets::addMaterial(const CgEngine::Assets::PackedMaterialAsset& material) {
        if (material.assetID == CgEngine::Assets::InvalidAssetID) {
            AP_LOGGING_WARNING("Trying to add PackedMaterialAsset with invalid asset ID.")
            return CgEngine::Assets::InvalidAssetID;
        }

        for (const auto& presentAsset : materials) {
            if (presentAsset.assetID == material.assetID) {
                return presentAsset.assetID;
            }
            if (presentAsset.data == material.data) {
                return presentAsset.assetID;
            }
        }

        materials.push_back(material);
        return material.assetID;
    }

    CgEngine::Assets::AssetID CollectedAssets::addTexture(const CgEngine::Assets::PackedTextureAsset& texture) {
        if (texture.assetID == CgEngine::Assets::InvalidAssetID) {
            AP_LOGGING_WARNING("Trying to add PackedTextureAsset with invalid asset ID.")
            return CgEngine::Assets::InvalidAssetID;
        }

        for (const auto& presentAsset : textures) {
            if (presentAsset.assetID == texture.assetID) {
                return presentAsset.assetID;
            }
            if (presentAsset.data == texture.data) {
                return presentAsset.assetID;
            }
        }

        textures.push_back(texture);
        return texture.assetID;
    }

    CgEngine::Assets::AssetID CollectedAssets::addImage(const CgEngine::Assets::PackedImageAsset& image) {
        if (image.assetID == CgEngine::Assets::InvalidAssetID) {
            AP_LOGGING_WARNING("Trying to add PackedImageAsset with invalid asset ID.")
            return CgEngine::Assets::InvalidAssetID;
        }

        for (const auto& presentAsset : images) {
            if (presentAsset.assetID == image.assetID) {
                return presentAsset.assetID;
            }
            if (presentAsset.name == image.name) {
                return presentAsset.assetID;
            }
        }

        images.push_back(image);
        return image.assetID;
    }

    CgEngine::Assets::AssetID CollectedAssets::addPhysicsMaterial(const CgEngine::Assets::PackedPhysicsMaterialAsset& material) {
        if (material.assetID == CgEngine::Assets::InvalidAssetID) {
            AP_LOGGING_WARNING("Trying to add PackedPhysicsMaterialAsset with invalid asset ID.")
            return CgEngine::Assets::InvalidAssetID;
        }

        for (const auto& presentAsset : physicsMaterials) {
            if (presentAsset.assetID == material.assetID) {
                return presentAsset.assetID;
            }
            if (presentAsset.data == material.data) {
                return presentAsset.assetID;
            }
        }

        physicsMaterials.push_back(material);
        return material.assetID;
    }

    CgEngine::Assets::AssetID CollectedAssets::getImageByName(const std::string& name) {
        for (const auto& presentAsset : images) {
            if (presentAsset.name == name) {
                return presentAsset.assetID;
            }
        }
        return CgEngine::Assets::InvalidAssetID;
    }

}
