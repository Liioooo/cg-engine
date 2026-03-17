#pragma once

#include "NumberComparisonUtils.h"
#include "glm/gtc/epsilon.hpp"

namespace CgEngine::NumberComparisonUtils {
    bool isFloatClose(float a, float b, float epsilon) {
        return std::fabs(a - b) < epsilon;
    }

    bool isVec2Close(const glm::vec3& a, const glm::vec3& b, float epsilon) {
        return glm::all(glm::epsilonEqual(a, b, epsilon));
    }

    bool isVec3Close(const glm::vec3& a, const glm::vec3& b, float epsilon) {
        return glm::all(glm::epsilonEqual(a, b, epsilon));
    }

    bool isVec4Close(const glm::vec3& a, const glm::vec3& b, float epsilon) {
        return glm::all(glm::epsilonEqual(a, b, epsilon));
    }
}
