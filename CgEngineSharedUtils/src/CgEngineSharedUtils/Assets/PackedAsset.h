#pragma once

#include "string"
#include "AssetId.h"

namespace CgEngine::Assets {

    template<typename T>
    struct PackedAsset {
        std::string name;
        AssetID assetID = InvalidAssetID;
        T data{};
    };

}
