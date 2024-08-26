#pragma once

#include "Asserts.h"

namespace CgEngine {

    class BoneInfluence {
    public:
        void addBoneData(uint32_t boneIndex, float weight);
        void normalizeWeights();

    private:
        glm::uvec4 boneIndices = {0u, 0u, 0u, 0u};
        glm::vec4 weights = {-1.0f, -1.0f, -1.0f, -1.0f};
    };
}
