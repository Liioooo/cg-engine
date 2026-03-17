#pragma once

#include "AssetIDGenerator.h"

namespace AssetPacker {
    std::random_device AssetIDGenerator::randomDevice;
    std::mt19937_64 AssetIDGenerator::eng(randomDevice());
    std::uniform_int_distribution<uint64_t> AssetIDGenerator::uniformDistribution;

    CgEngine::Assets::AssetID AssetIDGenerator::generateAssetID() {
        return uniformDistribution(eng);
    }
}
