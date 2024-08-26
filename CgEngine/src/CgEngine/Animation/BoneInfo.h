#pragma once

namespace CgEngine {
    class BoneInfo {
    public:
        BoneInfo(const glm::mat4& offset, uint32_t boneIndex, uint32_t submeshIndex);

        const glm::mat4 offset;
        const uint32_t boneIndex;
        const uint32_t submeshIndex;
    };
}