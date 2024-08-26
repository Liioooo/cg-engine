#pragma once

namespace CgEngine {

    class Uuid {
    public:
        Uuid();

        bool operator ==(const Uuid& other) const {
            return uuid == other.uuid;
        }

        uint32_t getUuid() const;

    private:
        uint32_t uuid;

        static std::random_device randomDevice;
        static std::mt19937_64 eng;
        static std::uniform_int_distribution<uint64_t> uniformDistribution;
    };

}
