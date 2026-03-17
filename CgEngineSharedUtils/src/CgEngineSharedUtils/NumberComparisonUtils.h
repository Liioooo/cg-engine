#pragma once

#include "glm/glm.hpp"

namespace CgEngine::NumberComparisonUtils {
    bool isFloatClose(float a, float b, float epsilon = 0.0001f);
    bool isVec2Close(const glm::vec3& a, const glm::vec3& b, float epsilon = 0.0001f);
    bool isVec3Close(const glm::vec3& a, const glm::vec3& b, float epsilon = 0.0001f);
    bool isVec4Close(const glm::vec3& a, const glm::vec3& b, float epsilon = 0.0001f);
}
