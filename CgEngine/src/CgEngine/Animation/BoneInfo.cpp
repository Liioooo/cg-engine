#pragma once

#include "BoneInfo.h"

namespace CgEngine {
    BoneInfo::BoneInfo(const glm::mat4& offset, uint32_t boneIndex, uint32_t submeshIndex) : offset(offset), boneIndex(boneIndex), submeshIndex(submeshIndex) {}
}