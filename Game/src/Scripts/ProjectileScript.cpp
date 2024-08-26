#include "ProjectileScript.h"
#include "GhostsController.h"

namespace Game {
    void ProjectileScript::update(CgEngine::TimeStep ts) {
        lifetime -= ts.getSeconds();

        if (lifetime <= 0.0f) {
            destroyEntity(getOwingEntity());
        }
    }

    void ProjectileScript::onCollisionEnter(CgEngine::Entity other) {
        if (getEntityTag(other) == "ghost") {
            auto& ghostsController = getComponent<CgEngine::ScriptComponent>(findEntityById("ghostsCoins")).getNativeScript<GhostsController>();
            ghostsController.notifyGhostHitByProjectile(other);
            destroyEntity(getOwingEntity());
        }
    }
}
