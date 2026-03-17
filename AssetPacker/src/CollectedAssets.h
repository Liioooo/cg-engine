#pragma once

#include "vector"
#include "CgEngineSharedUtils/Assets/MaterialAsset.h"
#include "CgEngineSharedUtils/Assets/TextureAsset.h"
#include "CgEngineSharedUtils/Assets/ImageAsset.h"
#include "CgEngineSharedUtils/Assets/PhysicsMaterialAsset.h"
#include "AssetPackerLogging.h"

namespace AssetPacker {

    struct CollectedAssets {
        std::vector<CgEngine::Assets::PackedMaterialAsset> materials;
        std::vector<CgEngine::Assets::PackedTextureAsset> textures;
        std::vector<CgEngine::Assets::PackedImageAsset> images;
        std::vector<CgEngine::Assets::PackedPhysicsMaterialAsset> physicsMaterials;

        // These functions add the asset if it doesn't already exist and return its AssetID
        // If it already exists, they return the AssetID of the existing asset
        CgEngine::Assets::AssetID addMaterial(const CgEngine::Assets::PackedMaterialAsset& material);
        CgEngine::Assets::AssetID addTexture(const CgEngine::Assets::PackedTextureAsset& texture);
        CgEngine::Assets::AssetID addImage(const CgEngine::Assets::PackedImageAsset& image);
        CgEngine::Assets::AssetID addPhysicsMaterial(const CgEngine::Assets::PackedPhysicsMaterialAsset & material);

        CgEngine::Assets::AssetID getImageByName(const std::string& name);
    };

}
