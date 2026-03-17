#pragma once

#include "PackedAsset.h"
#include "CgEngineSharedUtils/NumberComparisonUtils.h"

namespace CgEngine::Assets {

    struct MaterialAsset {
        glm::vec3 albedoColor = {1.0f, 1.0f, 1.0f};
        float metalness = 0.0f;
        float roughness = 1.0f;
        glm::vec3 emission = {0.0f, 0.0f, 0.0f};
        AssetID emissionTexture = InvalidAssetID;
        AssetID albedoTexture = InvalidAssetID;
        AssetID metalnessTexture = InvalidAssetID;
        AssetID roughnessTexture = InvalidAssetID;
        AssetID normalTexture = InvalidAssetID;

        bool operator==(const MaterialAsset& other) const {
            return NumberComparisonUtils::isVec3Close(albedoColor, other.albedoColor) &&
                   NumberComparisonUtils::isFloatClose(metalness, other.metalness) &&
                   NumberComparisonUtils::isFloatClose(roughness, other.roughness) &&
                   NumberComparisonUtils::isVec3Close(emission, other.emission) &&
                   emissionTexture == other.emissionTexture &&
                   albedoTexture == other.albedoTexture &&
                   metalnessTexture == other.metalnessTexture &&
                   roughnessTexture == other.roughnessTexture &&
                   normalTexture == other.normalTexture;
        };
    };

    using PackedMaterialAsset = PackedAsset<MaterialAsset>;

}
