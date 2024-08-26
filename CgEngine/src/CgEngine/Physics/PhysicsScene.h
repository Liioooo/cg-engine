#pragma once

#include "Scene/Entity.h"
#include "TimeStep.h"
#include "PhysicsActor.h"
#include "PxPhysicsAPI.h"
#include "PhysicsController.h"

namespace CgEngine {

    class Scene;

    class RaycastQueryFilterCallback : public physx::PxQueryFilterCallback {
    public:
        physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;

        const std::unordered_set<Entity>* excludedEntities;
    };

    class PhysicsScene {
    public:
        PhysicsScene();
        ~PhysicsScene();

        void addActor(PhysicsActor* actor);
        void removeActor(PhysicsActor* actor);
        PhysicsController* createController(Scene& scene, Entity entity, bool hasGravity, float stepOffset, float stepDownOffset, float slopeLimit);
        void simulate(TimeStep ts, Scene& scene);

        PhysicsRaycastHit raycast(glm::vec3 origin, glm::vec3 direction, float maxDistance, const std::unordered_set<Entity>& excludeEntities);

    private:
        physx::PxScene* physXScene;
        physx::PxControllerManager* physXControllerManager;

        RaycastQueryFilterCallback raycastQueryFilterCallback;

        bool advance(float ts, Scene& scene);
        void updateControllers(float ts);

        float simulateTimeStep;
        float accumulator;
    };

}
