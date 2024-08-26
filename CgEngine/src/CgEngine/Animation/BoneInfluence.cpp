#pragma once

#include "BoneInfluence.h"
#include "Asserts.h"

namespace CgEngine {
    void BoneInfluence::addBoneData(uint32_t boneIndex, float weight) {
        weight = glm::clamp(weight, 0.0f, 1.0f);

        for (int i = 0; i < 4; i++) {
            if (weights[i] < 0.0f) {
                boneIndices[i] = boneIndex;
                weights[i] = weight;
                return;
            }
        }

        CG_ASSERT(false, "One Vertex cannot be affected by more than 4 Bones")
    }

    void BoneInfluence::normalizeWeights() {
        float sum = 0.0f;
        for (int i = 0; i < 4; i++) {
            sum += weights[i] > 0.0f ? weights[i] : 0.0f;
        }
        if (sum > 0.0f) {
            for (int i = 0; i < 4; ++i) {
                weights[i] = weights[i] > 0.0f ? weights[i] / sum : 0.0f;
            }
        }
    }

}
