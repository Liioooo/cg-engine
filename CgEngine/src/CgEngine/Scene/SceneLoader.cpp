#include "SceneLoader.h"
#include "pugixml.hpp"
#include "Timer.h"

namespace CgEngine {
    Scene* SceneLoader::loadScene(XMLFile* xmlSceneFile, int viewportWidth, int viewportHeight) {
        CG_LOGGING_DEBUG("Loading Scene")

        const pugi::xml_document& xml = xmlSceneFile->getXMLDocument();
        auto* scene = new Scene(viewportWidth, viewportHeight);
        const auto& sceneNode = xml.child("Scene");
        for (const auto &item: sceneNode.children()) {
            createEntity(scene, NoEntity, item);
        }

        CG_LOGGING_DEBUG("Loaded Scene")
        return scene;
    }

    void SceneLoader::createEntity(Scene *scene, Entity parent, const pugi::xml_node& node) {
        const auto& idAttr = node.attribute("id");
        const auto& tagAttr = node.attribute("tag");
        Entity entity;
        if (parent == NoEntity) {
            entity = idAttr.empty() ? scene->createEntity() : scene->createEntity(idAttr.as_string());
        } else {
            entity = idAttr.empty() ? scene->createEntity(parent) : scene->createEntity(parent, idAttr.as_string());
        }
        if (!tagAttr.empty()) {
            scene->setEntityTag(entity, tagAttr.as_string());
        }

        const auto& componentsNode = node.child("Components");
        for (const auto &compNode: componentsNode.children()) {
            createComponent(scene, entity, compNode);
        }
        CG_ASSERT(scene->hasComponent<TransformComponent>(entity), "Every entity must contain a TransformComponent!")

        for (const auto &child: node.children("Entity")) {
            createEntity(scene, entity, child);
        }
    }

    void SceneLoader::createComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        CG_TIME_FN_INFO(node.name())

        std::string name = node.name();
        if (name == "TransformComponent") {
            createTransformComponent(scene, entity, node);
        } else if (name == "MeshRendererComponent") {
            createMeshRendererComponent(scene, entity, node);
        } else if (name == "AnimatedMeshRendererComponent") {
            createAnimatedMeshRendererComponent(scene, entity, node);
        } else if (name == "CameraComponent") {
            createCameraComponent(scene, entity, node);
        } else if (name == "ScriptComponent") {
            createScriptComponent(scene, entity, node);
        } else if (name == "DirectionalLightComponent") {
            createDirectionalLightComponent(scene, entity, node);
        } else if (name == "PointLightComponent") {
            createPointLightComponent(scene, entity, node);
        } else if (name == "SpotLightComponent") {
            createSpotLightComponent(scene, entity, node);
        } else if (name == "SkyboxComponent") {
            createSkyboxComponent(scene, entity, node);
        } else if (name == "RigidBodyComponent") {
            createRigidBodyComponent(scene, entity, node);
        } else if (name == "BoxColliderComponent") {
            createBoxColliderComponent(scene, entity, node);
        } else if (name == "SphereColliderComponent") {
            createSphereColliderComponent(scene, entity, node);
        } else if (name == "CapsuleColliderComponent") {
            createCapsuleColliderComponent(scene, entity, node);
        } else if (name == "TriangleColliderComponent") {
            createTriangleColliderComponent(scene, entity, node);
        } else if (name == "ConvexColliderComponent") {
            createConvexColliderComponent(scene, entity, node);
        } else if (name == "CharacterControllerComponent") {
            createCharacterControllerComponent(scene, entity, node);
        } else if (name == "UiCanvasComponent") {
            createUiCanvasComponent(scene, entity, node);
        }
    }

     void SceneLoader::createTransformComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        glm::vec3 position = stringTupleToVec3(node.attribute("position").as_string());
        glm::vec3 rotation = stringTupleToVec3(node.attribute("rotation").as_string());
        glm::vec3 scale = stringTupleToVec3(node.attribute("scale").as_string());
        TransformComponentParams params{position, glm::radians(rotation), scale};
        scene->attachComponent<TransformComponent>(entity, params);
    }

    void SceneLoader::createMeshRendererComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        MeshRendererComponentParams params{
            node.attribute("asset-file").as_string(),
            node.attribute("mesh").as_string(),
            node.attribute("material").as_string(),
            node.attribute("cast-shadows").as_bool(true),
            getListFromString(node.attribute("mesh-nodes").as_string())
        };
        scene->attachComponent<MeshRendererComponent>(entity, params);
    }

    void SceneLoader::createAnimatedMeshRendererComponent(CgEngine::Scene* scene, CgEngine::Entity entity, const pugi::xml_node& node) {
        AnimatedMeshRendererComponentParams params{
                node.attribute("asset-file").as_string(),
                node.attribute("material").as_string(),
                node.attribute("cast-shadows").as_bool(true),
                getListFromString(node.attribute("mesh-nodes").as_string()),
                node.attribute("animation").as_string(""),
                node.attribute("animation-speed").as_float(1.0f),
                node.attribute("auto-play").as_bool(true),
                node.attribute("loop").as_bool(true)
        };
        scene->attachComponent<AnimatedMeshRendererComponent>(entity, params);
    }

    void SceneLoader::createCameraComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        CameraComponentParams params{
            node.attribute("projection").as_string("perspective"),
            node.attribute("near").as_float(0.1f),
            node.attribute("far").as_float(100.0f),
            node.attribute("fov").as_float(60.0f),
            node.attribute("ortho-size").as_float(10.0f),
            node.attribute("primary").as_bool(false),
            node.attribute("exposure").as_float(1.0f),
            node.attribute("bloom-intensity").as_float(1.0f),
            node.attribute("bloom-threshold").as_float(0.2f),

        };
        scene->attachComponent<CameraComponent>(entity, params);
    }

    void SceneLoader::createScriptComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        ScriptComponentParams params{
            node.attribute("script-name").as_string()
        };
        scene->attachComponent<ScriptComponent>(entity, params);
    }

    void SceneLoader::createDirectionalLightComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        DirectionalLightComponentParams params{
                hexStringToColor(node.attribute("color").as_string("1 1 1")),
                node.attribute("intensity").as_float(1.0f),
                node.attribute("cast-shadows").as_bool(true)
        };
        scene->attachComponent<DirectionalLightComponent>(entity, params);
    }

    void SceneLoader::createPointLightComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        PointLightComponentParams params{
                hexStringToColor(node.attribute("color").as_string("1 1 1")),
                node.attribute("intensity").as_float(1.0f),
                node.attribute("radius").as_float(5.0f),
                node.attribute("falloff").as_float(1.0f),
        };
        scene->attachComponent<PointLightComponent>(entity, params);
    }

    void SceneLoader::createSpotLightComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        SpotLightComponentParams params{
                hexStringToColor(node.attribute("color").as_string("1 1 1")),
                node.attribute("intensity").as_float(1.0f),
                node.attribute("radius").as_float(5.0f),
                node.attribute("falloff").as_float(1.0f),
                glm::radians(node.attribute("inner-angle").as_float(30.0f)),
                glm::radians(node.attribute("outer-angle").as_float(35.0f)),
        };
        scene->attachComponent<SpotLightComponent>(entity, params);
    }

    void SceneLoader::createSkyboxComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        SkyboxComponentParams params{
            node.attribute("hdri-path").as_string(),
            node.attribute("intensity").as_float(1.0f),
            node.attribute("lod").as_float(1.0f)
        };
        scene->attachComponent<SkyboxComponent>(entity, params);
    }

    void SceneLoader::createRigidBodyComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        RigidBodyComponentParams params{
            node.attribute("dynamic").as_bool(false),
            node.attribute("kinematic").as_bool(false),
            node.attribute("disable-gravity").as_bool(false),
            node.attribute("mass").as_float(1.0f),
            node.attribute("linear-drag").as_float(0.0f),
            node.attribute("angular-drag").as_float(0.0f),
            std::strcmp(node.attribute("collision-detection").as_string("discrete"), "discrete") ? PhysicsCollisionDetection::Discrete : PhysicsCollisionDetection::Continuous,
        };
        scene->attachComponent<RigidBodyComponent>(entity, params);
    }

    void SceneLoader::createBoxColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        BoxColliderComponentParams params{
            stringTupleToVec3(node.attribute("half-size").as_string("0.5 0.5 0.5")),
            stringTupleToVec3(node.attribute("offset").as_string("0 0 0")),
            node.attribute("trigger").as_bool(false),
            node.attribute("material").as_string("default-physics-material"),
        };
        scene->attachComponent<BoxColliderComponent>(entity, params);
    }

    void SceneLoader::createSphereColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        SphereColliderComponentParams params{
            node.attribute("radius").as_float(1.0f),
            stringTupleToVec3(node.attribute("offset").as_string("0 0 0")),
            node.attribute("trigger").as_bool(false),
            node.attribute("material").as_string("default-physics-material"),
        };
        scene->attachComponent<SphereColliderComponent>(entity, params);
    }

    void SceneLoader::createCapsuleColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        CapsuleColliderComponentParams params{
            node.attribute("radius").as_float(1.0f),
            node.attribute("half-height").as_float(0.5f),
            stringTupleToVec3(node.attribute("offset").as_string("0 0 0")),
            node.attribute("trigger").as_bool(false),
            node.attribute("material").as_string("default-physics-material"),
        };
        scene->attachComponent<CapsuleColliderComponent>(entity, params);
    }

    void SceneLoader::createTriangleColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        TriangleColliderComponentParams params{
                node.attribute("asset-file").as_string(""),
                node.attribute("mesh-node").as_string(""),
                node.attribute("trigger").as_bool(false),
                node.attribute("material").as_string("default-physics-material"),
        };
        scene->attachComponent<TriangleColliderComponent>(entity, params);
    }

    void SceneLoader::createConvexColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        ConvexColliderComponentParams params{
                node.attribute("asset-file").as_string(""),
                node.attribute("mesh-node").as_string(""),
                node.attribute("trigger").as_bool(false),
                node.attribute("material").as_string("default-physics-material"),
        };
        scene->attachComponent<ConvexColliderComponent>(entity, params);
    }

    void SceneLoader::createCharacterControllerComponent(CgEngine::Scene* scene, CgEngine::Entity entity, const pugi::xml_node& node) {
        CharacterControllerComponentParams params{
            node.attribute("has-gravity").as_bool(true),
            node.attribute("step-offset").as_float(0.0f),
            node.attribute("step-down-offset").as_float(0.0f),
            node.attribute("slope-limit").as_float(0.0f)
        };
        scene->attachComponent<CharacterControllerComponent>(entity, params);
    }

    void SceneLoader::createUiCanvasComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        UiCanvasComponentParams params{
            node.children()
        };
        scene->attachComponent<UiCanvasComponent>(entity, params);
    }

    glm::vec3 SceneLoader::stringTupleToVec3(const std::string& s) {
        size_t p0 = 0;
        size_t p1 = s.find(' ');
        float x = std::stof(s.substr(p0, p1));
        p0 = p1 + 1;
        p1 = s.find(' ', p0);
        float y = std::stof(s.substr(p0, p1));
        float z = std::stof(s.substr(p1 + 1));

        return {x, y, z};
    }

    glm::vec3 SceneLoader::hexStringToColor(const std::string& s) {
        uint64_t color = std::stoul(s.substr(1), nullptr, 16);
        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8) & 0xFF) / 255.0f;
        float b = (color & 0xFF) / 255.0f;
        return {r, g, b};
    }

    std::vector<std::string> SceneLoader::getListFromString(const std::string& s) {
        std::vector<std::string> result{};
        std::stringstream ss(s);
        std::string item;

        while (std::getline(ss, item, ',')) {
            result.emplace_back(item);
        }

        return result;
    }
}
