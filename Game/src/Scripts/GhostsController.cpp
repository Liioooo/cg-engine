#include "GhostsController.h"
#include "glm/gtx/vector_angle.hpp"

namespace Game {
    void GhostsController::onAttach() {
        /*
         * 28--32--------------29--33
         * |    |               |   |
         * 30--24--------------20--27
         * |    |               |   |
         * 22--14-----21-------12--18
         * |    |      |        |   |
         * |    |  2---0---1    |   |
         * |    |  |       |    |   |
         * 13---8--4-------3----6--10
         * |    |  |       |    |   |
         * |    |  7---9---5    |   |
         * |    |      |        |   |
         * 23--15-----16-------11--17
         * |    |               |   |
         * 31--25--------------19--26
         */

        mapNodes.emplace_back(glm::vec3(0.0f, 1.0f, -17.0f));   // 0
        mapNodes.emplace_back(glm::vec3(9.0f, 1.0f, -17.0f));   // 1
        mapNodes.emplace_back(glm::vec3(-9.0f, 1.0f, -17.0f));  // 2
        mapNodes.emplace_back(glm::vec3(9.0f, 1.0f, 0.0f));     // 3
        mapNodes.emplace_back(glm::vec3(-9.0f, 1.0f, 0.0f));    // 4
        mapNodes.emplace_back(glm::vec3(9.0f, 1.0f, 12.0f));    // 5
        mapNodes.emplace_back(glm::vec3(17.0f, 1.0f, 0.0f));    // 6
        mapNodes.emplace_back(glm::vec3(-9.0f, 1.0f, 12.0f));   // 7
        mapNodes.emplace_back(glm::vec3(-17.0f, 1.0f, 0.0f));   // 8
        mapNodes.emplace_back(glm::vec3(0.0f, 1.0f, 12.0f));    // 9
        mapNodes.emplace_back(glm::vec3(31.5f, 1.0f, 0.0f));    // 10
        mapNodes.emplace_back(glm::vec3(17.0f, 1.0f, 32.0f));   // 11
        mapNodes.emplace_back(glm::vec3(17.0f, 1.0f, -32.0f));  // 12
        mapNodes.emplace_back(glm::vec3(-31.5f, 1.0f, 0.0f));   // 13
        mapNodes.emplace_back(glm::vec3(-17.0f, 1.0f, -32.0f)); // 14
        mapNodes.emplace_back(glm::vec3(-17.0f, 1.0f, 32.0f));  // 15
        mapNodes.emplace_back(glm::vec3(0.0f, 1.0f, 32.0f));    // 16
        mapNodes.emplace_back(glm::vec3(31.5f, 1.0f, 32.0f));   // 17
        mapNodes.emplace_back(glm::vec3(31.5f, 1.0f, -32.0f));  // 18
        mapNodes.emplace_back(glm::vec3(17.0f, 1.0f, 46.5f));   // 19
        mapNodes.emplace_back(glm::vec3(17.0f, 8.5f, -54.0f));  // 20
        mapNodes.emplace_back(glm::vec3(0.0f, 1.0f, -32.0f));   // 21
        mapNodes.emplace_back(glm::vec3(-31.5f, 1.0f, -32.0f)); // 22
        mapNodes.emplace_back(glm::vec3(-31.5f, 1.0f, 32.0f));  // 23
        mapNodes.emplace_back(glm::vec3(-17.0f, 8.5f, -54.0f)); // 24
        mapNodes.emplace_back(glm::vec3(-17.0f, 1.0f, 46.5f));  // 25
        mapNodes.emplace_back(glm::vec3(31.5f, 1.0f, 46.5f));   // 26
        mapNodes.emplace_back(glm::vec3(31.5f, 8.5f, -54.0f));  // 27
        mapNodes.emplace_back(glm::vec3(-31.5f, 8.5f, -66.0f)); // 28
        mapNodes.emplace_back(glm::vec3(17.0f, 8.5f, -66.0f));  // 29
        mapNodes.emplace_back(glm::vec3(-31.5f, 8.5f, -54.0f)); // 30
        mapNodes.emplace_back(glm::vec3(-31.5f, 1.0f, 46.5f));  // 31
        mapNodes.emplace_back(glm::vec3(-17.0f, 8.5f, -66.0f)); // 32
        mapNodes.emplace_back(glm::vec3(31.5f, 8.5f, -66.0f));  // 33

        setMapNeighbors(0, {1, 2, 21});
        setMapNeighbors(1, {0, 3});
        setMapNeighbors(2, {0, 4});
        setMapNeighbors(3, {1, 4, 5, 6});
        setMapNeighbors(4, {2, 3, 7, 8});
        setMapNeighbors(5, {3, 9});
        setMapNeighbors(6, {3, 10, 11, 12});
        setMapNeighbors(7, {4, 9});
        setMapNeighbors(8, {4, 13, 14, 15});
        setMapNeighbors(9, {5, 7, 16});
        setMapNeighbors(10, {6, 17, 18});
        setMapNeighbors(11, {6, 16, 17, 19});
        setMapNeighbors(12, {6, 18, 20, 21});
        setMapNeighbors(13, {8, 22, 23});
        setMapNeighbors(14, {21, 22, 24});
        setMapNeighbors(15, {8, 16, 23, 25});
        setMapNeighbors(16, {11, 15});
        setMapNeighbors(17, {10, 11, 26});
        setMapNeighbors(18, {10, 12, 27});
        setMapNeighbors(19, {11, 25, 26});
        setMapNeighbors(20, {12, 24, 27, 29});
        setMapNeighbors(21, {0, 12, 14});
        setMapNeighbors(22, {13, 14, 30});
        setMapNeighbors(23, {13, 15, 31});
        setMapNeighbors(24, {14, 20, 30, 32});
        setMapNeighbors(25, {15, 19, 31});
        setMapNeighbors(26, {17, 19});
        setMapNeighbors(27, {18, 20, 33});
        setMapNeighbors(28, {30, 32});
        setMapNeighbors(29, {20, 32, 33});
        setMapNeighbors(30, {22, 24, 28});
        setMapNeighbors(31, {23, 25});
        setMapNeighbors(32, {24, 28, 29});
        setMapNeighbors(33, {27, 29});

        createCoins();
        createGhosts();
    }

    void GhostsController::update(CgEngine::TimeStep ts) {
        for (auto& g: ghosts) {
            if (g.state == GhostState::Home) {
                timeSinceGhostLeavingHome += ts.getSeconds();

                if (timeSinceGhostLeavingHome >= 3.0f) {
                    g.state = GhostState::LeavingHome;
                    timeSinceGhostLeavingHome = 0.0f;
                }
                break;
            }
        }
    }

    void GhostsController::fixedUpdate(CgEngine::TimeStep ts) {
        CgEngine::Entity playerEntity = findEntityById("player");
        glm::vec3 playerPos = getComponent<CgEngine::TransformComponent>(playerEntity).getGlobalPosition();

        for (auto& g: ghosts) {
            if (g.state == GhostState::Dead) {
                getComponent<CgEngine::RigidBodyComponent>(g.entity).setGlobalPose(g.homePos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
                auto& meshTransform = getComponent<CgEngine::TransformComponent>(*getChildEntities(g.entity).cbegin());
                meshTransform.setLocalRotationVec({0.0f, glm::pi<float>(), 0.0f});
                g.state = GhostState::Home;
                timeSinceGhostLeavingHome = 0.0f;
                continue;
            }

            if (g.state == GhostState::LeavingHome) {
                getComponent<CgEngine::RigidBodyComponent>(g.entity).setKinematicTarget(g.homePos + glm::vec3(0.0f, 0.0f, 2.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
                g.state = GhostState::Moving;
                if (random(0, 1) == 0) {
                    g.lastMapNode = 3;
                    g.nextMapNode = 4;
                } else {
                    g.lastMapNode = 4;
                    g.nextMapNode = 3;
                }
                continue;
            }

            if (g.state == GhostState::Moving) {
                auto& rigid = getComponent<CgEngine::RigidBodyComponent>(g.entity);
                if (glm::distance(rigid.getGlobalPosePosition(), mapNodes[g.nextMapNode].pos) < 0.4f) {
                    // if they can see the player chase him
                    bool chasePlayer = false;
                    glm::vec3 playerInDir = glm::normalize(playerPos - mapNodes[g.nextMapNode].pos);
                    float playerDistance = glm::distance(playerPos, mapNodes[g.nextMapNode].pos);
                    auto raycastHit = physicsRaycast(mapNodes[g.nextMapNode].pos, playerInDir, playerDistance, {g.entity, playerEntity});
                    if (!raycastHit.hitFound) {
                        float smallestAngle = 99.0f;
                        int smallestAngleIndex = -1;
                        for (int i: mapNodes[g.nextMapNode].neighbors) {
                            float angle = glm::angle(playerInDir, glm::normalize(mapNodes[i].pos - mapNodes[g.nextMapNode].pos));
                            if (angle < smallestAngle) {
                                smallestAngle = angle;
                                smallestAngleIndex = i;
                            }
                        }

                        if (smallestAngle < glm::radians(10.0f)) {
                            g.lastMapNode = g.nextMapNode;
                            g.nextMapNode = smallestAngleIndex;
                            chasePlayer = true;
                        }
                    }

                    if (!chasePlayer) {
                        uint32_t numNeighbors = mapNodes[g.nextMapNode].neighbors.size();
                        uint32_t nextIndex = random(0, numNeighbors - 1);

                        // prevent going back
                        if (mapNodes[g.nextMapNode].neighbors[nextIndex] == g.lastMapNode) {
                            nextIndex = nextIndex == numNeighbors - 1 ? 0 : nextIndex + 1;
                        }

                        // prevent going to a node, where another ghost is already going
                        if (hasAnyGhostNextNode(mapNodes[g.nextMapNode].neighbors[nextIndex])) {
                            if (numNeighbors > 2) {
                                nextIndex = nextIndex == numNeighbors - 1 ? 0 : nextIndex + 1;
                            }
                        }

                        g.lastMapNode = g.nextMapNode;
                        g.nextMapNode = mapNodes[g.nextMapNode].neighbors[nextIndex];
                    }
                }

                glm::vec3 direction = glm::normalize(mapNodes[g.nextMapNode].pos - rigid.getGlobalPosePosition());
                rigid.setKinematicTarget(rigid.getGlobalPosePosition() + direction * ts.getSeconds() * 6.0f, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));

                auto rot = glm::atan(direction.x, direction.z) - glm::pi<float>();

                auto& meshTransform = getComponent<CgEngine::TransformComponent>(*getChildEntities(g.entity).cbegin());
                meshTransform.setLocalRotationVec({0.0f, rot, 0.0f});
            }
        }
    }

    uint32_t GhostsController::getTotalCoinAmount() {
        return totalCoinAmount;
    }

    void GhostsController::notifyGhostHitByProjectile(CgEngine::Entity ghostEntity) {
        for (auto& g: ghosts) {
            if (g.entity == ghostEntity ) {
                g.lives--;
                if (g.lives == 0) {
                    g.state = GhostState::Dead;
                    g.lives = 5;
                }
            }
        }
    }

    void GhostsController::setMapNeighbors(size_t node, std::initializer_list<int> neighbors) {
        for (const auto& n: neighbors) {
            mapNodes[node].neighbors.push_back(n);
        }
    }

    void GhostsController::createCoins() {
        CgEngine::Entity coinContainer = createEntity();
        attachComponent<CgEngine::TransformComponent>(coinContainer, CgEngine::TransformComponentParams{glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f)});

        std::unordered_set<std::string> visitedEdges;
        for (int i = 0; i < mapNodes.size(); ++i) {
            createCoinAtPos(coinContainer, mapNodes[i].pos);
            for (int j = 0; j < mapNodes[i].neighbors.size(); ++j) {
                int jIndex = mapNodes[i].neighbors[j];
                std::string edgeKey = i < jIndex ? std::to_string(i)  + ":" + std::to_string(jIndex) : std::to_string(jIndex)  + ":" + std::to_string(i);
                if (visitedEdges.find(edgeKey) != visitedEdges.end()) {
                    continue;
                }
                visitedEdges.insert(edgeKey);

                float edgeLength = glm::distance(mapNodes[i].pos, mapNodes[jIndex].pos);
                int coinAmount = static_cast<int>(glm::round(edgeLength / 1.5f));

                for (int c = 1; c < coinAmount; c++) {
                    createCoinAtPos(coinContainer, lerp(mapNodes[i].pos, mapNodes[jIndex].pos, static_cast<float>(c) / static_cast<float>(coinAmount)));
                }
            }
        }
    }

    void GhostsController::createCoinAtPos(CgEngine::Entity container, glm::vec3 pos) {
        CgEngine::Entity e = createEntity(container);
        setEntityTag(e, "coin");

        pos.y -= 0.5f;

        CgEngine::RigidBodyComponentParams rigidBodyParams{};
        rigidBodyParams.isDynamic = false;

        attachComponent<CgEngine::TransformComponent>(e, CgEngine::TransformComponentParams{pos, glm::vec3(0.0f), glm::vec3(0.1f)});
        attachComponent<CgEngine::MeshRendererComponent>(e, CgEngine::MeshRendererComponentParams{"", "CG_SphereMesh", "Coins", true});
        attachComponent<CgEngine::SphereColliderComponent>(e, CgEngine::SphereColliderComponentParams{1.4f, glm::vec3(0.0f), true, "default-physics-material"});
        attachComponent<CgEngine::RigidBodyComponent>(e, rigidBodyParams);

        totalCoinAmount++;
    }

    void GhostsController::createGhosts() {
        CgEngine::Entity ghostContainer = createEntity();
        attachComponent<CgEngine::TransformComponent>(ghostContainer, CgEngine::TransformComponentParams{glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f)});

        ghosts[0].homePos = glm::vec3(-4.0f, 1.0f, -2.0f);
        ghosts[1].homePos = glm::vec3(-2.0f, 1.0f, -2.0f);
        ghosts[2].homePos = glm::vec3(0.0f, 1.0f, -2.0f);
        ghosts[3].homePos = glm::vec3(2.0f, 1.0f, -2.0f);
        ghosts[4].homePos = glm::vec3(4.0f, 1.0f, -2.0f);

        CgEngine::RigidBodyComponentParams rigidBodyParams{};
        rigidBodyParams.isDynamic = true;
        rigidBodyParams.isKinematic = true;

        for (uint32_t i = 0; i < ghosts.size(); i++) {
            uint32_t ghostModel = i % 3;
            std::string ghostModelStr = std::to_string(ghostModel + 1);

            auto& g = ghosts[i];
            g.entity = createEntity(ghostContainer);
            setEntityTag(g.entity, "ghost");
            attachComponent<CgEngine::TransformComponent>(g.entity, CgEngine::TransformComponentParams{g.homePos, glm::vec3(0.0f), glm::vec3(1.0f, 0.8f, 1.0f)});
            attachComponent<CgEngine::ConvexColliderComponent>(g.entity, CgEngine::ConvexColliderComponentParams{"models.fbx", "Ghost" + ghostModelStr, false, "default-physics-material"});
            attachComponent<CgEngine::RigidBodyComponent>(g.entity, rigidBodyParams);

            glm::vec3 lightColor;

            switch (ghostModel) {
                case 0:
                    lightColor = {1.0f, 0.05f, 0.05f};
                    break;
                case 1:
                    lightColor = {0.2f, 1.0f, 0.1f};
                    break;
                case 2:
                    lightColor = {0.05f, 0.05f, 1.0f};
                    break;
            }

            attachComponent<CgEngine::PointLightComponent>(g.entity, CgEngine::PointLightComponentParams{lightColor, 1.0f, 4.0f, 0.8f});

            auto meshEntity = createEntity(g.entity);
            attachComponent<CgEngine::TransformComponent>(meshEntity, CgEngine::TransformComponentParams{glm::vec3(0.0f), glm::vec3(0.0f, glm::pi<float>(), 0.0f), glm::vec3(1.0f)});
            attachComponent<CgEngine::AnimatedMeshRendererComponent>(meshEntity, CgEngine::AnimatedMeshRendererComponentParams{"models.fbx", "", true, {"Ghost" + ghostModelStr}, "ArmatureGhost" + ghostModelStr + "|" + "Ghost" + ghostModelStr + "Anim", 2.0f, true, true});
        }
    }

    glm::vec3 GhostsController::lerp(glm::vec3 x, glm::vec3 y, float t) {
        return x * (1.0f - t) + y * t;
    }

    uint32_t GhostsController::random(uint32_t min, uint32_t max) {
        std::random_device randomDevice;
        std::mt19937_64 eng(randomDevice());
        std::uniform_int_distribution<uint32_t> uniformDistribution(min, max);
        return uniformDistribution(eng);
    }

    bool GhostsController::hasAnyGhostNextNode(uint32_t nodeId) {
        for (const auto& g: ghosts) {
            if (g.nextMapNode == nodeId) {
                return true;
            }
        }
        return false;
    }

    void GhostsController::debugDrawMap() {
        for (const auto& n1: mapNodes) {
            for (const auto& n2: n1.neighbors) {
                drawDebugLine(n1.pos, mapNodes[n2].pos, {1.0f, 0.0f, 0.0f});
            }
        }
    }
}
