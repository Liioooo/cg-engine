#pragma once

#include "CgEngine/Scripting/NativeScript.h"
#include "CgEngine/Logging.h"

namespace Game {

    struct MapNode {
        glm::vec3 pos;
        std::vector<int> neighbors;

        explicit MapNode(glm::vec3 pos) : pos(pos) {};
    };

    enum class GhostState {
        Dead, Home, LeavingHome, Moving
    };

    struct Ghost {
        glm::vec3 homePos;
        GhostState state = GhostState::Home;
        int nextMapNode = -1;
        int lastMapNode = -1;
        int lives = 5;
        CgEngine::EntityHandle entity;
    };

    class GhostsController : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<GhostsController>();
        }

        uint32_t getTotalCoinAmount();
        void notifyGhostHitByProjectile(CgEngine::Entity ghostEntity);

    protected:
        void onAttach() override;
        void update(CgEngine::TimeStep ts) override;

        void fixedUpdate(CgEngine::TimeStep ts) override;

    private:
        std::vector<MapNode> mapNodes;
        std::array<Ghost, 5> ghosts;
        uint32_t totalCoinAmount = 0;

        float timeSinceGhostLeavingHome = 0.0f;

        void setMapNeighbors(size_t node, std::initializer_list<int> neighbors);
        void createCoins();
        void createCoinAtPos(CgEngine::Entity container, glm::vec3 pos);
        void createGhosts();

        glm::vec3 lerp(glm::vec3 x, glm::vec3 y, float t);
        uint32_t random(uint32_t min, uint32_t max);
        bool hasAnyGhostNextNode(uint32_t nodeId);

        void debugDrawMap();
    };

}
