#include "Material.h"
#include "GraphicsObjectsFactory.h"

namespace CgEngine {

    Material::Material() : uuid(Uuid()) {
        pushConstants = GraphicsObjectsFactory::createPushConstants("pc_material");
        descriptorSet = GraphicsObjectsFactory::createDescriptorSet();
    }

    Material::~Material() {
        delete descriptorSet;
        delete pushConstants;
    }

    const Uuid& Material::getUuid() const {
        return uuid;
    }

    bool Material::operator==(const CgEngine::Material& other) const {
        return uuid == other.uuid;
    }

    DescriptorSet* Material::getDescriptorSet() const {
        return descriptorSet;
    }

    PushConstants* Material::getPushConstants() const {
        return pushConstants;
    }


}
