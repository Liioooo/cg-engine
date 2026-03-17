#pragma once

#include "PackedAsset.h"
#include "vector"

namespace CgEngine::Assets {

    struct ImageAsset {
        std::vector<uint8_t> imageData;
    };

    using PackedImageAsset = PackedAsset<ImageAsset>;

}
