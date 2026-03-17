#pragma once

#include <random>
#include "CgEngineSharedUtils/Assets/AssetId.h"
#include "CgEngineSharedUtils/Assets/PackedAsset.h"
#include "AssetPackerLogging.h"

namespace AssetPacker {
    class AssetIDGenerator {
    public:
        static CgEngine::Assets::AssetID generateAssetID();

        template<typename T>
        static void assignAssetIdToPackedAsset(CgEngine::Assets::PackedAsset<T>& packedAsset) {
            if (packedAsset.assetID != CgEngine::Assets::InvalidAssetID) {
                AP_LOGGING_WARNING("Packed asset already has an AssetID assigned. Overwriting existing AssetID.");
            }
            packedAsset.assetID = generateAssetID();
        }

    private:
        static std::random_device randomDevice;
        static std::mt19937_64 eng;
        static std::uniform_int_distribution<uint64_t> uniformDistribution;
    };
}
