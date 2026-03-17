#include "ProjectileScript.h"
#include "GhostsController.h"
#include "CgEngine/Components/ScriptComponent.h"

namespace Game {
    void ProjectileScript::update(CgEngine::TimeStep ts) {
        lifetime -= ts.getSeconds();

        if (lifetime <= 0.0f) {
            getOwingEntity().destroy();
        }
    }

    void ProjectileScript::onCollisionEnter(CgEngine::EntityHandle other) {
        if (other.getTag() == "ghost") {
            auto& ghostsController = findEntityById("ghostsCoins").getComponent<CgEngine::ScriptComponent>()->getNativeScript<GhostsController>();
            ghostsController.notifyGhostHitByProjectile(other);
            getOwingEntity().destroy();
        }
    }
}
