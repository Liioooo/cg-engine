#pragma once

#include "Uuid.h"
#include "DescriptorSet.h"

namespace CgEngine {

    class Material {
    public:
        Material();
        virtual ~Material();

        const Uuid& getUuid() const;

        bool operator ==(const Material& other) const;

        DescriptorSet* getDescriptorSet() const;

    protected:
        DescriptorSet* descriptorSet = nullptr;

    private:
        Uuid uuid;
    };

}
