#pragma once

#include <cstdint>
#include <utility>
#include "Enums.h"

namespace CgEngine::UIPosUtils {
    uint32_t convertUIPosToPixels(std::pair<float, UIPosUnit> uiPos, float referenceWidth, float referenceHeight);
}
