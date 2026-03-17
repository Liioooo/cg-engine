#include "PhysicsMaterial.h"
#include "FileSystem.h"
#include "Application.h"
#include "CgEngineSharedUtils/StringUtils.h"

namespace CgEngine {
    PhysicsMaterial* PhysicsMaterial::createResource(const std::string& name) {
        if (name == "default-physics-material") {
            return new PhysicsMaterial(0.6f, 0.6f, 0.0f);
        }

        if (!physicsMaterialsXML.isLoaded()) {
            physicsMaterialsXML.load(FileSystem::getAsGamePath("physics-materials.xml"));
        }

        const pugi::xml_document& materialsXML = physicsMaterialsXML.getXMLDocument();
        const auto& materials = materialsXML.child("Materials");
        const auto& materialNode = materials.find_child_by_attribute("Material", "name", name.c_str());
        std::string staticFriction = materialNode.child("StaticFriction").child_value();
        std::string dynamicFriction = materialNode.child("DynamicFriction").child_value();
        std::string restitution = materialNode.child("Restitution").child_value();

        return new PhysicsMaterial(staticFriction.empty() ? 0.0f : StringUtils::toFloat(staticFriction).value_or(0.0f), dynamicFriction.empty() ? 0.0f : StringUtils::toFloat(dynamicFriction).value_or(0.0f), restitution.empty() ? 0.0f : StringUtils::toFloat(restitution).value_or(0.0f));
    }

    PhysicsMaterial::PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution) : staticFriction(staticFriction), dynamicFriction(dynamicFriction), restitution(restitution) {
        auto& physxPhysics = Application::get().getPhysicsSystem().getPhysxPhysics();

        physxMaterial = physxPhysics.createMaterial(staticFriction, dynamicFriction, restitution);
    }

    PhysicsMaterial::~PhysicsMaterial() {
        physxMaterial->release();
        physxMaterial = nullptr;
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
