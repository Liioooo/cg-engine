#pragma once

#include "Scene.h"
#include "XMLFile.h"

namespace CgEngine {

    class SceneLoader {
        friend class PrefabManager;
    public:
        static Scene* loadScene(XMLFile& xmlSceneFile, int viewportWidth, int viewportHeight);

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
        static void createUiCanvasComponent2D(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createAnimationComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createCustomShaderRendererComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createAudioListenerComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createAudioComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
        static void createLodDistanceComponent(Scene* scene, Entity entity, const pugi::xml_node& node);
    };

}
