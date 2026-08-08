#include "SceneLoader.h"
#include "pugixml.hpp"
#include "Timer.h"
#include "CgEngineSharedUtils/LoaderUtils.h"
#include "CgEngineSharedUtils/StringUtils.h"
#include "PrefabManager.h"
#include "EntityHandle.h"
#include "Components/AnimationComponent.h"
#include "Components/TransformComponent.h"
#include "Components/ScriptComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/AnimatedMeshRendererComponent.h"
#include "Components/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkyboxComponent.h"
#include "Components/BoxColliderComponent.h"
#include "Components/SphereColliderComponent.h"
#include "Components/CapsuleColliderComponent.h"
#include "Components/TriangleColliderComponent.h"
#include "Components/ConvexColliderComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/CharacterControllerComponent.h"
#include "Components/UiCanvasComponent2D.h"
#include "Components/AudioListenerComponent.h"
#include "Components/AudioComponent.h"
#include "Components/LodDistanceComponent.h"
#include "Components/CustomShaderRendererComponent.h"

namespace CgEngine {
    Scene* SceneLoader::loadScene(XMLFile& xmlSceneFile, int viewportWidth, int viewportHeight) {
        CG_LOGGING_DEBUG("Loading Scene")

        const pugi::xml_document& xml = xmlSceneFile.getXMLDocument();
        auto* scene = new Scene(viewportWidth, viewportHeight);
        const auto& sceneNode = xml.child("Scene");
        for (const auto &item: sceneNode.children("Entity")) {
            createEntity(scene, NoEntity, item);
        }
        scene->executeAllPendingOperations(true);

        CG_LOGGING_DEBUG("Loaded Scene")
        return scene;
    }

    void SceneLoader::createEntity(Scene *scene, Entity parent, const pugi::xml_node& node) {
        const auto& idAttr = node.attribute("id");
        const auto& tagAttr = node.attribute("tag");
        EntityHandle entity = idAttr.empty() ? scene->createEntity(parent) : scene->createEntity(parent, idAttr.as_string());
        if (!tagAttr.empty()) {
            entity.setTag(tagAttr.as_string());
        }

        const auto& componentsNode = node.child("Components");
        const auto& transformNode = componentsNode.child("TransformComponent");

        createTransformComponent(scene, entity, transformNode);

        for (const auto &compNode: componentsNode.children()) {
            if (std::string_view(compNode.name()) == "TransformComponent") {
                continue;
            }
            createComponent(scene, entity, compNode);
        }

        for (const auto &child: node.children("Entity")) {
            createEntity(scene, entity, child);
        }

        for (const auto &child: node.children("Prefab")) {
            PrefabManager::instantiatePrefabFromNode(scene, child, entity);
        }
    }

    void SceneLoader::createComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        CG_TIME_FN_INFO(node.name())

        std::string_view name = node.name();
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
        } else if (name == "UiCanvasComponent2D") {
            createUiCanvasComponent2D(scene, entity, node);
        } else if (name == "AnimationComponent") {
            createAnimationComponent(scene, entity, node);
        } else if (name == "CustomShaderRendererComponent") {
            createCustomShaderRendererComponent(scene, entity, node);
        } else if (name == "AudioListenerComponent") {
            createAudioListenerComponent(scene, entity, node);
        } else if (name == "AudioComponent") {
            createAudioComponent(scene, entity, node);
        } else if (name == "LodDistanceComponent") {
            createLodDistanceComponent(scene, entity, node);
        }
    }

     void SceneLoader::createTransformComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        bool rotRads = false;

        TransformComponentParams params;
        if (!node.attribute("position").empty()) params.position = LoaderUtils::stringTupleToVec3(node.attribute("position").as_string());
        if (!node.attribute("rotation-rads").empty()) rotRads = node.attribute("rotation-rads").as_bool();
        if (!node.attribute("rotation").empty()) {
            params.rotation = LoaderUtils::stringTupleToVec3(node.attribute("rotation").as_string());
            if (!rotRads) {
                params.rotation = glm::radians(params.rotation);
            }
        }
        if (!node.attribute("scale").empty()) params.scale = LoaderUtils::stringTupleToVec3(node.attribute("scale").as_string());

        scene->attachComponent<TransformComponent>(entity, params);
    }

    void SceneLoader::createMeshRendererComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        MeshRendererComponentParams params;
        if (!node.attribute("asset-file").empty()) params.assetFile = node.attribute("asset-file").as_string();
        if (!node.attribute("mesh").empty()) params.mesh = node.attribute("mesh").as_string();
        if (!node.attribute("material").empty()) params.material = node.attribute("material").as_string();
        if (!node.attribute("cast-shadows").empty()) params.castShadows = node.attribute("cast-shadows").as_bool();
        if (!node.attribute("enable-culling").empty()) params.enableCulling = node.attribute("enable-culling").as_bool();
        if (!node.attribute("mesh-nodes").empty()) params.meshNodes = LoaderUtils::getListFromString(node.attribute("mesh-nodes").as_string());

        scene->attachComponent<MeshRendererComponent>(entity, params);
    }

    void SceneLoader::createAnimatedMeshRendererComponent(CgEngine::Scene* scene, CgEngine::Entity entity, const pugi::xml_node& node) {
        AnimatedMeshRendererComponentParams params;
        if (!node.attribute("asset-file").empty()) params.assetFile = node.attribute("asset-file").as_string();
        if (!node.attribute("material").empty()) params.material = node.attribute("material").as_string();
        if (!node.attribute("cast-shadows").empty()) params.castShadows = node.attribute("cast-shadows").as_bool();
        if (!node.attribute("mesh-nodes").empty()) params.meshNodes = LoaderUtils::getListFromString(node.attribute("mesh-nodes").as_string());
        if (!node.attribute("animation").empty()) params.animation = node.attribute("animation").as_string("");
        if (!node.attribute("animation-speed").empty()) params.animationSpeed = node.attribute("animation-speed").as_float();
        if (!node.attribute("animation-start-time").empty()) params.animationStartTime = node.attribute("animation-start-time").as_float();
        if (!node.attribute("auto-play").empty()) params.autoPlayAnimation = node.attribute("auto-play").as_bool();
        if (!node.attribute("loop").empty()) params.loopAnimation = node.attribute("loop").as_bool();

        scene->attachComponent<AnimatedMeshRendererComponent>(entity, params);
    }

    void SceneLoader::createCameraComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        CameraComponentParams params;
        if (!node.attribute("projection").empty()) params.projection = node.attribute("projection").as_string();
        if (!node.attribute("near").empty()) params.cnear = node.attribute("near").as_float();
        if (!node.attribute("far").empty()) params.cfar = node.attribute("far").as_float();
        if (!node.attribute("fov").empty()) params.cfov = node.attribute("fov").as_float();
        if (!node.attribute("ortho-size").empty()) params.orthoSize = node.attribute("ortho-size").as_float();
        if (!node.attribute("primary").empty()) params.isPrimary = node.attribute("primary").as_bool();
        if (!node.attribute("exposure").empty()) params.exposure = node.attribute("exposure").as_float();

        if (!node.attribute("bloom-intensity").empty()) params.bloomIntensity = node.attribute("bloom-intensity").as_float();
        if (!node.attribute("bloom-threshold").empty()) params.bloomThreshold = node.attribute("bloom-threshold").as_float();

        if (!node.attribute("hbao-radius").empty()) params.hbaoRadius = node.attribute("hbao-radius").as_float();
        if (!node.attribute("hbao-intensity").empty()) params.hbaoIntensity = node.attribute("hbao-intensity").as_float();
        if (!node.attribute("hbao-bias").empty()) params.hbaoBias = node.attribute("hbao-bias").as_float();
        if (!node.attribute("hbao-sharpness").empty()) params.hbaoSharpness = node.attribute("hbao-sharpness").as_float();

        scene->attachComponent<CameraComponent>(entity, params);
    }

    void SceneLoader::createScriptComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        ScriptComponentParams params;
        if (!node.attribute("script-name").empty()) params.scriptName = node.attribute("script-name").as_string();

        for (const auto& attribute: node.attributes()) {
            params.parameterMap.insert(attribute.name(), attribute.as_string());
        }

        scene->attachComponent<ScriptComponent>(entity, params);
    }

    void SceneLoader::createDirectionalLightComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        DirectionalLightComponentParams params;
        if (!node.attribute("color").empty()) params.color = LoaderUtils::hexStringToColor(node.attribute("color").as_string());
        if (!node.attribute("intensity").empty()) params.intensity = node.attribute("intensity").as_float();
        if (!node.attribute("cast-shadows").empty()) params.castShadows = node.attribute("cast-shadows").as_bool();

        scene->attachComponent<DirectionalLightComponent>(entity, params);
    }

    void SceneLoader::createPointLightComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        PointLightComponentParams params;
        if (!node.attribute("color").empty()) params.color = LoaderUtils::hexStringToColor(node.attribute("color").as_string());
        if (!node.attribute("intensity").empty()) params.intensity = node.attribute("intensity").as_float();
        if (!node.attribute("radius").empty()) params.radius = node.attribute("radius").as_float();
        if (!node.attribute("falloff").empty()) params.falloff = node.attribute("falloff").as_float();

        scene->attachComponent<PointLightComponent>(entity, params);
    }

    void SceneLoader::createSpotLightComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        SpotLightComponentParams params;
        if (!node.attribute("color").empty()) params.color = LoaderUtils::hexStringToColor(node.attribute("color").as_string("1 1 1"));
        if (!node.attribute("intensity").empty()) params.intensity = node.attribute("intensity").as_float(1.0f);
        if (!node.attribute("radius").empty()) params.radius = node.attribute("radius").as_float(5.0f);
        if (!node.attribute("falloff").empty()) params.falloff = node.attribute("falloff").as_float(1.0f);
        if (!node.attribute("inner-angle").empty()) params.innerAngle = glm::radians(node.attribute("inner-angle").as_float(30.0f));
        if (!node.attribute("outer-angle").empty()) params.outerAngle = glm::radians(node.attribute("outer-angle").as_float(35.0f));

        scene->attachComponent<SpotLightComponent>(entity, params);
    }

    void SceneLoader::createSkyboxComponent(Scene *scene, Entity entity, const pugi::xml_node &node) {
        SkyboxComponentParams params;
        if (!node.attribute("hdri-path").empty()) params.hdriPath = node.attribute("hdri-path").as_string();
        if (!node.attribute("intensity").empty()) params.intensity = node.attribute("intensity").as_float();
        if (!node.attribute("lod").empty()) params.lod = node.attribute("lod").as_float();

        scene->attachComponent<SkyboxComponent>(entity, params);
    }

    void SceneLoader::createRigidBodyComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        RigidBodyComponentParams params;
        if (!node.attribute("dynamic").empty()) params.isDynamic = node.attribute("dynamic").as_bool();
        if (!node.attribute("kinematic").empty()) params.isKinematic = node.attribute("kinematic").as_bool();
        if (!node.attribute("disable-gravity").empty()) params.disableGravity=  node.attribute("disable-gravity").as_bool();
        if (!node.attribute("mass").empty()) params.mass = node.attribute("mass").as_float();
        if (!node.attribute("linear-drag").empty()) params.linearDrag = node.attribute("linear-drag").as_float();
        if (!node.attribute("angular-drag").empty()) params.angularDrag = node.attribute("angular-drag").as_float();
        if (!node.attribute("collision-detection").empty()) params.collisionDetection = std::string_view(node.attribute("collision-detection").as_string()) == "discrete" ? PhysicsCollisionDetection::Discrete : PhysicsCollisionDetection::Continuous;

        scene->attachComponent<RigidBodyComponent>(entity, params);
    }

    void SceneLoader::createBoxColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        BoxColliderComponentParams params;
        if (!node.attribute("half-size").empty()) params.halfSize = LoaderUtils::stringTupleToVec3(node.attribute("half-size").as_string());
        if (!node.attribute("offset").empty()) params.offset = LoaderUtils::stringTupleToVec3(node.attribute("offset").as_string());
        if (!node.attribute("trigger").empty()) params.isTrigger = node.attribute("trigger").as_bool();
        if (!node.attribute("material").empty()) params.material = node.attribute("material").as_string();

        scene->attachComponent<BoxColliderComponent>(entity, params);
    }

    void SceneLoader::createSphereColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        SphereColliderComponentParams params;
        if (!node.attribute("radius").empty()) params.radius = node.attribute("radius").as_float();
        if (!node.attribute("offset").empty()) params.offset = LoaderUtils::stringTupleToVec3(node.attribute("offset").as_string());
        if (!node.attribute("trigger").empty()) params.isTrigger = node.attribute("trigger").as_bool();
        if (!node.attribute("material").empty()) params.material = node.attribute("material").as_string();

        scene->attachComponent<SphereColliderComponent>(entity, params);
    }

    void SceneLoader::createCapsuleColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        CapsuleColliderComponentParams params;
        if (!node.attribute("radius").empty()) params.radius = node.attribute("radius").as_float();
        if (!node.attribute("half-height").empty()) params.halfHeight = node.attribute("half-height").as_float();
        if (!node.attribute("offset").empty()) params.offset = LoaderUtils::stringTupleToVec3(node.attribute("offset").as_string());
        if (!node.attribute("trigger").empty()) params.isTrigger = node.attribute("trigger").as_bool();
        if (!node.attribute("material").empty()) params.material = node.attribute("material").as_string();

        scene->attachComponent<CapsuleColliderComponent>(entity, params);
    }

    void SceneLoader::createTriangleColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        TriangleColliderComponentParams params;
        if (!node.attribute("asset-file").empty()) params.assetFile = node.attribute("asset-file").as_string();
        if (!node.attribute("mesh-node").empty()) params.meshNode = node.attribute("mesh-node").as_string();
        if (!node.attribute("trigger").empty()) params.isTrigger = node.attribute("trigger").as_bool();
        if (!node.attribute("material").empty()) params.material = node.attribute("material").as_string();

        scene->attachComponent<TriangleColliderComponent>(entity, params);
    }

    void SceneLoader::createConvexColliderComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        ConvexColliderComponentParams params;
        if (!node.attribute("asset-file").empty()) params.assetFile = node.attribute("asset-file").as_string();
        if (!node.attribute("mesh-node").empty()) params.meshNode = node.attribute("mesh-node").as_string();
        if (!node.attribute("trigger").empty()) params.isTrigger = node.attribute("trigger").as_bool();
        if (!node.attribute("material").empty()) params.material = node.attribute("material").as_string();

        scene->attachComponent<ConvexColliderComponent>(entity, params);
    }

    void SceneLoader::createCharacterControllerComponent(CgEngine::Scene* scene, CgEngine::Entity entity, const pugi::xml_node& node) {
        CharacterControllerComponentParams params;
        if (!node.attribute("has-gravity").empty()) params.hasGravity = node.attribute("has-gravity").as_bool();
        if (!node.attribute("step-offset").empty()) params.stepOffset = node.attribute("step-offset").as_float();
        if (!node.attribute("step-down-offset").empty()) params.stepDownOffset = node.attribute("step-down-offset").as_float();
        if (!node.attribute("slope-limit").empty()) params.slopeLimit = node.attribute("slope-limit").as_float();

        scene->attachComponent<CharacterControllerComponent>(entity, params);
    }

    void SceneLoader::createUiCanvasComponent2D(Scene* scene, Entity entity, const pugi::xml_node& node) {
        UiCanvasComponent2DParams params;
        params.canvasName = node.attribute("canvas").as_string();
        if (!node.attribute("pos-x").empty()) params.posX = LoaderUtils::stringToUIPosAndUnit(node.attribute("pos-x").as_string());
        if (!node.attribute("pos-y").empty()) params.posY = LoaderUtils::stringToUIPosAndUnit(node.attribute("pos-y").as_string());
        if (!node.attribute("width").empty()) params.width = LoaderUtils::stringToUIPosAndUnit(node.attribute("width").as_string());
        if (!node.attribute("height").empty()) params.height = LoaderUtils::stringToUIPosAndUnit(node.attribute("height").as_string());
        if (!node.attribute("x-align").empty()) params.xAlignment = LoaderUtils::stringToUIXAlignment(node.attribute("x-align").as_string());
        if (!node.attribute("y-align").empty()) params.yAlignment = LoaderUtils::stringToUIYAlignment(node.attribute("y-align").as_string());
        if (!node.attribute("receive-input").empty()) params.receiveInputEvents = node.attribute("receive-input").as_bool();
        if (!node.attribute("z-index").empty()) params.zIndex = node.attribute("z-index").as_uint(0u);

        scene->attachComponent<UiCanvasComponent2D>(entity, params);
    }

    void SceneLoader::createAnimationComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        AnimationComponentParams params;
        if (!node.attribute("asset-file").empty()) params.assetFile = node.attribute("asset-file").as_string();
        if (!node.attribute("animation").empty()) params.animation = node.attribute("animation").as_string("");
        if (!node.attribute("animation-speed").empty()) params.animationSpeed = node.attribute("animation-speed").as_float();
        if (!node.attribute("auto-play").empty()) params.autoPlayAnimation = node.attribute("auto-play").as_bool();
        if (!node.attribute("loop").empty()) params.loopAnimation = node.attribute("loop").as_bool();

        scene->attachComponent<AnimationComponent>(entity, params);
    }

    void SceneLoader::createCustomShaderRendererComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        CustomShaderRendererComponentParams params;
        if (!node.attribute("asset-file").empty()) params.assetFile = node.attribute("asset-file").as_string();
        if (!node.attribute("mesh").empty()) params.mesh = node.attribute("mesh").as_string();
        if (!node.attribute("enable-culling").empty()) params.enableCulling = node.attribute("enable-culling").as_bool();
        if (!node.attribute("bounding-min").empty()) params.boundingMin = LoaderUtils::stringTupleToVec3(node.attribute("bounding-min").as_string());
        if (!node.attribute("bounding-max").empty()) params.boundingMax = LoaderUtils::stringTupleToVec3(node.attribute("bounding-max").as_string());
        if (!node.attribute("mesh-nodes").empty()) params.meshNodes = LoaderUtils::getListFromString(node.attribute("mesh-nodes").as_string());
        if (!node.attribute("instance-count").empty()) params.instanceCount = node.attribute("instance-count").as_uint();

        scene->attachComponent<CustomShaderRendererComponent>(entity, params);
    }

    void SceneLoader::createAudioListenerComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        AudioListenerComponentParams params;
        if (!node.attribute("active").empty()) params.active = node.attribute("active").as_bool();
        if (!node.attribute("volume").empty()) params.volume = node.attribute("volume").as_float();

        scene->attachComponent<AudioListenerComponent>(entity, params);
    }

    void SceneLoader::createAudioComponent(Scene* scene, Entity entity, const pugi::xml_node& node) {
        AudioComponentParams params;
        if (!node.attribute("asset-file").empty()) params.assetFile = node.attribute("asset-file").as_string();
        if (!node.attribute("volume").empty()) params.volume = node.attribute("volume").as_float();
        if (!node.attribute("pitch").empty()) params.pitch = node.attribute("pitch").as_float();
        if (!node.attribute("looping").empty()) params.looping = node.attribute("looping").as_bool();
        if (!node.attribute("play-on-attach").empty()) params.playOnAttach = node.attribute("play-on-attach").as_bool();
        if (!node.attribute("auto-destroy").empty()) params.autoDestroy = node.attribute("auto-destroy").as_bool();

        scene->attachComponent<AudioComponent>(entity, params);
    }

    void SceneLoader::createLodDistanceComponent(CgEngine::Scene* scene, CgEngine::Entity entity, const pugi::xml_node& node) {
        LodDistanceComponentParams params;

        if (!node.attribute("lod-distances").empty()) {
            std::vector<std::string> lodDistances = LoaderUtils::getListFromString(node.attribute("lod-distances").as_string());
            for (const auto& lodDistance: lodDistances) {
                params.lodDistances.emplace_back(StringUtils::toFloat(lodDistance).value_or(0.0f));
            }
        }

        scene->attachComponent<LodDistanceComponent>(entity, params);
    }
}
