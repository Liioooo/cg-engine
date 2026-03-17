#pragma once

#include "PackedAsset.h"
#include "CgEngineSharedUtils/Enums.h"

namespace CgEngine::Assets {

    struct TextureAsset {
        TextureWrap wrap = TextureWrap::Repeat;
        MipMapFiltering mipMapFiltering = MipMapFiltering::Anisotropic;
        bool srgb = false;
        AssetID image = InvalidAssetID;

        bool operator==(const TextureAsset& other) const {
            return wrap == other.wrap &&
                   mipMapFiltering == other.mipMapFiltering &&
                   srgb == other.srgb &&
                   image == other.image;
        };
    };

    using PackedTextureAsset = PackedAsset<TextureAsset>;

}
