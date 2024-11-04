#include "MemoryBarrierBit.h"

namespace CgEngine {

    unsigned int MemoryBarrierUtils::convertToBitfield(const std::initializer_list<MemoryBarrierBit>& barriers) {
        unsigned int barrier = 0;
        for (const auto& b: barriers) {
            barrier &= static_cast<unsigned int>(b);
        }
        return barrier;
    }
}
