#include "UIPosUtils.h"

namespace CgEngine::UIPosUtils {

    uint32_t convertUIPosToPixels(std::pair<float, UIPosUnit> uiPos, float referenceWidth, float referenceHeight) {
        switch (uiPos.second) {
            case UIPosUnit::Pixel:
                return static_cast<uint32_t>(uiPos.first);
            case UIPosUnit::VWPercent:
                return static_cast<uint32_t>(static_cast<float>(referenceWidth) * uiPos.first);
            case UIPosUnit::VHPercent:
                return static_cast<uint32_t>(static_cast<float>(referenceHeight) * uiPos.first);
        }
    }

}
