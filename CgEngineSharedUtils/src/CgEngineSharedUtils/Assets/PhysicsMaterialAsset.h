#pragma once

#include "PackedAsset.h"
#include "CgEngineSharedUtils/NumberComparisonUtils.h"

namespace CgEngine::Assets {

    struct PhysicsMaterialAsset {
        float staticFriction = 0.5f;
        float dynamicFriction = 0.5f;
        float restitution = 0.0f;

        bool operator==(const PhysicsMaterialAsset& other) const {
            return NumberComparisonUtils::isFloatClose(staticFriction, other.staticFriction) &&
                   NumberComparisonUtils::isFloatClose(dynamicFriction, other.dynamicFriction) &&
                   NumberComparisonUtils::isFloatClose(restitution, other.restitution);
        };
    };

    using PackedPhysicsMaterialAsset = PackedAsset<PhysicsMaterialAsset>;

}
