#include "PhysicsMaterial.h"
#include "GlobalObjectManager.h"
#include "FileSystem.h"

namespace CgEngine {
    PhysicsMaterial* PhysicsMaterial::createResource(const std::string& name) {
        if (name == "default-physics-material") {
            return new PhysicsMaterial(0.6f, 0.6f, 0.0f);
        }

        auto& resourceManager = GlobalObjectManager::getInstance().getResourceManager();

        const pugi::xml_document& materialsXML = resourceManager.getResource<XMLFile>(FileSystem::getAsGamePath("physics-materials.xml"))->getXMLDocument();
        const auto& materials = materialsXML.child("Materials");
        const auto& materialNode = materials.find_child_by_attribute("Material", "name", name.c_str());
        std::string staticFriction = materialNode.child("StaticFriction").child_value();
        std::string dynamicFriction = materialNode.child("DynamicFriction").child_value();
        std::string restitution = materialNode.child("Restitution").child_value();

        return new PhysicsMaterial(staticFriction.empty() ? 0.0f : std::stof(staticFriction), dynamicFriction.empty() ? 0.0f : std::stof(dynamicFriction), restitution.empty() ? 0.0f : std::stof(restitution));
    }

    PhysicsMaterial::PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution) : staticFriction(staticFriction), dynamicFriction(dynamicFriction), restitution(restitution) {
        auto& physxPhysics = GlobalObjectManager::getInstance().getPhysicsSystem().getPhysxPhysics();

        physxMaterial = physxPhysics.createMaterial(staticFriction, dynamicFriction, restitution);
    }

    float PhysicsMaterial::getStaticFriction() const {
        return staticFriction;
    }

    float PhysicsMaterial::getDynamicFriction() const {
        return dynamicFriction;
    }

    float PhysicsMaterial::getRestitution() const {
        return restitution;
    }

    physx::PxMaterial* PhysicsMaterial::getPhysxMaterial() {
        return physxMaterial;
    }
}
