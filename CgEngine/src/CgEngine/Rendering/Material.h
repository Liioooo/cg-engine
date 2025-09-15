#pragma once

#include "Uuid.h"
#include "DescriptorSet.h"
#include "PushConstants.h"

namespace CgEngine {

    class Material {
    public:
        Material();
        virtual ~Material();

        const Uuid& getUuid() const;

        bool operator ==(const Material& other) const;

        DescriptorSet* getDescriptorSet() const;
        PushConstants* getPushConstants() const;

    protected:
        DescriptorSet* descriptorSet = nullptr;
        PushConstants* pushConstants = nullptr;

    private:
        Uuid uuid;
    };

}
