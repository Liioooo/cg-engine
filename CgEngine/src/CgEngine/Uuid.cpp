#include "Uuid.h"

namespace CgEngine {
    std::random_device Uuid::randomDevice;
    std::mt19937_64 Uuid::eng(randomDevice());
    std::uniform_int_distribution<uint64_t> Uuid::uniformDistribution;

    Uuid::Uuid() : uuid(uniformDistribution(eng)) {}

    uint32_t Uuid::getUuid() const {
        return uuid;
    }
}
