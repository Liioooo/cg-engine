#pragma once

#include "Scene.h"
#include "Resources/XMLFile.h"

namespace CgEngine {

    class SceneLoader {
    public:
        static Scene* loadScene(XMLFile* xmlSceneFile, int viewportWidth, int viewportHeight);

    private:
        static void createEntity(Scene* scene, Entity parent, const pugi::xml_node& node);
        static void createComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createTransformComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createMeshRendererComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createAnimatedMeshRendererComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createCameraComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createScriptComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createDirectionalLightComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createPointLightComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createSpotLightComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createSkyboxComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createRigidBodyComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createBoxColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createSphereColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createCapsuleColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createTriangleColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createConvexColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createCharacterControllerComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createUiCanvasComponent(Scene* scene, Entity entity, const pugi::xml_node& node);

        static glm::vec3 stringTupleToVec3(const std::string& s);
        static glm::vec3 hexStringToColor(const std::string& s);
        static std::vector<std::string> getListFromString(const std::string& s);

    };

}
