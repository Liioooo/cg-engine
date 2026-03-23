#include "Scene.h"
#include "Asserts.h"
#include "Rendering/SceneRenderer.h"
#include "Application.h"
#include "Audio/AudioComponentUpdateData.h"
#include "EntityHandle.h"
#include "Components/TransformComponent.h"
#include "Components/ScriptComponent.h"
#include "Components/UiCanvasComponent2D.h"
#include "Components/AnimationComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/AnimatedMeshRendererComponent.h"
#include "Components/AudioComponent.h"
#include "Components/AudioListenerComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/LodDistanceComponent.h"
#include "Components/SkyboxComponent.h"
#include "Components/BoxColliderComponent.h"
#include "Components/SphereColliderComponent.h"
#include "Components/CapsuleColliderComponent.h"
#include "Components/TriangleColliderComponent.h"
#include "Components/ConvexColliderComponent.h"
#include "Components/CustomShaderRendererComponent.h"

namespace CgEngine {
    Scene::Scene(int viewportWidth, int viewportHeight) : viewportWidth(viewportWidth), viewportHeight(viewportHeight) {}

    Scene::~Scene() {
        componentManager.destroyAllComponents(*this);
    }

    EntityHandle Scene::createEntity(Entity parent) {
        if (parent == NoEntity) {
            Entity entity = nextEntityId++;
            children[entity] = std::unordered_set<Entity>();
            return {this, entity};
        }
        if (!hasEntity(parent)) {
            CG_ASSERT(false, "Entity parent doesn't exist")
            return {this, NoEntity};
        }

        Entity entity = nextEntityId++;
        children[parent].insert(entity);
        children[entity] = std::unordered_set<Entity>();
        parents[entity] = parent;
        return {this, entity};
    }

    EntityHandle Scene::createEntity(Entity parent, const std::string& id) {
        if (idToEntity.find(id) != idToEntity.end()) {
            CG_ASSERT(false, "Entity-Id is already used")
            return {this, NoEntity};
        }

        if (parent == NoEntity) {
            Entity entity = nextEntityId++;
            idToEntity[id] = entity;
            children[entity] = std::unordered_set<Entity>();
            return {this, entity};
        }
        if (!hasEntity(parent)) {
            CG_ASSERT(false, "Entity parent doesn't exist")
            return {this, NoEntity};
        }

        Entity entity = nextEntityId++;
        idToEntity[id] = entity;
        children[parent].insert(entity);
        children[entity] = std::unordered_set<Entity>();
        parents[entity] = parent;
        return {this, entity};
    }

    void Scene::destroyEntity(Entity entity) {
        entitiesToBeDestroyed.insert(entity);
    }

    EntityHandle Scene::findEntityById(const std::string &id) {
        if (idToEntity.find(id) == idToEntity.end()) {
            CG_LOGGING_WARNING("Entity with ID: {0} doesn't exist!", id)
            return {this, NoEntity};
        }
        return {this, idToEntity.at(id)};
    }

    std::optional<std::string> Scene::getIdForEntity(Entity entity) const {
        auto idIter = std::find_if(
                std::begin(idToEntity),
                std::end(idToEntity),
                [entity](auto&& p) { return p.second == entity;}
        );
        if (idIter != std::end(idToEntity)) {
            return idIter->first;
        }
        return std::nullopt;
    }

    const std::unordered_set<Entity>& Scene::getChildren(Entity entity) const {
        return children.at(entity);
    }

    Entity Scene::getParent(Entity entity) const {
        if (parents.find(entity) == parents.end()) {
            return NoEntity;
        }
        return parents.at(entity);
    }

    bool Scene::hasParent(Entity entity) const {
        return parents.find(entity) != parents.end();
    }

    bool Scene::hasEntity(Entity entity) const {
        return children.find(entity) != children.end();
    }

    void Scene::setEntityTag(Entity entity, const std::string& tag) {
        if (hasEntity(entity)) {
            entityTags[entity] = tag;
        }
    }

    std::string Scene::getEntityTag(Entity entity) const {
        if (entityTags.find(entity) != entityTags.end()) {
            return entityTags.at(entity);
        }
        return "";
    }

    void Scene::updateTransforms() {
        for (const auto &[entity, _] : children) {
            if (parents.find(entity) == parents.end()) {
                auto& topLevelTransform = componentManager.getComponent<TransformComponent>(entity);
                bool topLevelDirty = topLevelTransform._calculateTopLevelTransforms();
                const glm::mat4& topLevelModelMatrix = topLevelTransform.getModelMatrix();

                for (const auto &child : children[entity]) {
                    recursiveUpdateChildTransforms(child, topLevelModelMatrix, topLevelDirty);
                }
            }
        }
    }

    void Scene::onViewportResize(int width, int height) {
        if (width == viewportWidth && height == viewportHeight) {
            return;
        }
        viewportWidth = width;
        viewportHeight = height;

        for (auto it = componentManager.begin<CameraComponent>(); it != componentManager.end<CameraComponent>(); it++) {
            it->getCamera().setViewportSize(width, height);
        }
    }

    void Scene::executeOnRender(const std::function<void(SceneRenderer&)>& function) {
        onRenderFunctions.emplace_back(function);
    }

    void Scene::onUpdate(TimeStep ts) {
        physicsScene.simulate(ts, *this);

        for (auto it = componentManager.begin<ScriptComponent>(); it != componentManager.end<ScriptComponent>(); it++) {
            it->update(ts);
        }
        for (auto it = componentManager.begin<AnimationComponent>(); it != componentManager.end<AnimationComponent>(); it++) {
            it->update(ts, componentManager.getComponent<TransformComponent>(it->getEntity()));
        }
        executeAllPendingOperations(true);

        for (auto it = componentManager.begin<ScriptComponent>(); it != componentManager.end<ScriptComponent>(); it++) {
            it->lateUpdate(ts);
        }
        executeAllPendingOperations(true);

        for (auto it = componentManager.begin<UiCanvasComponent2D>(); it != componentManager.end<UiCanvasComponent2D>(); it++) {
            it->update(viewportWidth, viewportHeight);
        }

        for (auto it = componentManager.begin<AnimatedMeshRendererComponent>(); it != componentManager.end<AnimatedMeshRendererComponent>(); it++) {
            if (it->isActive()) it->update(ts);
        }

        bool usePrimaryCameraAsListener = true;
        if (!componentManager.getEntitiesWithComponent<AudioListenerComponent>().empty()) {
            for (auto it = componentManager.begin<AudioListenerComponent>(); it != componentManager.end<AudioListenerComponent>(); it++) {
                if (it->isActive()) {
                    usePrimaryCameraAsListener = false;
                    auto& audioSystem = AudioSystem::get();

                    auto& transform = componentManager.getComponent<TransformComponent>(it->getEntity());
                    audioSystem.updateListenerPosition({transform.getGlobalRotationQuat(), transform.getGlobalPosition()});
                    audioSystem.updateListenerVolume(it->getVolume());
                    if (componentManager.hasComponent<RigidBodyComponent>(it->getEntity())) {
                        auto& rigidBody = componentManager.getComponent<RigidBodyComponent>(it->getEntity());
                        if (rigidBody.isDynamic()) {
                            audioSystem.updateListenerVelocity(rigidBody.getLinearVelocity());
                        }
                    }
                    break;
                }
            }
        }
        if (usePrimaryCameraAsListener) {
            auto& primaryCamera = getPrimaryCamaraComponent();
            auto& audioSystem = AudioSystem::get();

            auto& transform = componentManager.getComponent<TransformComponent>(primaryCamera.getEntity());
            audioSystem.updateListenerPosition({transform.getGlobalRotationQuat(), transform.getGlobalPosition()});
            audioSystem.updateListenerVolume(1.0f);
            if (componentManager.hasComponent<RigidBodyComponent>(primaryCamera.getEntity())) {
                auto& rigidBody = componentManager.getComponent<RigidBodyComponent>(primaryCamera.getEntity());
                if (rigidBody.isDynamic()) {
                    audioSystem.updateListenerVelocity(rigidBody.getLinearVelocity());
                }
            }
        }

        const auto audioComponentsMarkedForDestroy = AudioSystem::get().getComponentsMarkedForDestroy();
        std::vector<Entity> audioComponentsToDestroy;
        audioComponentsToDestroy.reserve(audioComponentsMarkedForDestroy.size());

        std::vector<AudioComponentUpdateData> audioComponentUpdateData;
        for (auto it = componentManager.begin<AudioComponent>(); it != componentManager.end<AudioComponent>(); it++) {
            if (audioComponentsMarkedForDestroy.find(it->getUuid()) != audioComponentsMarkedForDestroy.end()) {
                audioComponentsToDestroy.emplace_back(it->getEntity());
                continue;
            }

            auto& transform = componentManager.getComponent<TransformComponent>(it->getEntity());

            auto& updateData = audioComponentUpdateData.emplace_back();
            updateData.uuid = it->getUuid();
            updateData.looping = it->isLooping();
            updateData.volume = it->getVolume();
            updateData.pitch = it->getPitch();
            updateData.transform = {transform.getGlobalRotationQuat(), transform.getGlobalPosition()};

            if (componentManager.hasComponent<RigidBodyComponent>(it->getEntity())) {
                auto& rigidBody = componentManager.getComponent<RigidBodyComponent>(it->getEntity());
                if (rigidBody.isDynamic()) {
                    updateData.velocity = rigidBody.getLinearVelocity();
                }
            }
        }
        AudioSystem::get().updateAudioComponents(std::move(audioComponentUpdateData));

        for (const auto& entity: audioComponentsToDestroy) {
            detachComponent<AudioComponent>(entity);
        }
        executeAllPendingOperations(false);
    }

    void Scene::onEvent(Event& event) {
        for (auto it = componentManager.begin<UiCanvasComponent2D>(); it != componentManager.end<UiCanvasComponent2D>(); it++) {
           it->onEvent(event, viewportWidth, viewportHeight);
        }

        for (auto it = componentManager.begin<ScriptComponent>(); it != componentManager.end<ScriptComponent>(); it++) {
            it->onEvent(event);
            if (event.wasHandled()) {
                return;
            }
        }
        executeAllPendingOperations(false);
    }

    void Scene::onRender(SceneRenderer& renderer) {
        auto& cameraComponent = getPrimaryCamaraComponent();

        auto cameraTransform = componentManager.getComponent<TransformComponent>(cameraComponent.getEntity());

        SceneLightEnvironment lightEnvironment{};

        auto dirLightComponentIt = componentManager.cbegin<DirectionalLightComponent>();
        if (dirLightComponentIt != componentManager.cend<DirectionalLightComponent>()) {
            lightEnvironment.dirLightDirection = glm::normalize(glm::mat3(componentManager.getComponent<TransformComponent>(dirLightComponentIt->getEntity()).getModelMatrix()) * glm::vec3(0.0f, 1.0f, 0.0f));
            lightEnvironment.dirLightColor = dirLightComponentIt->getColor();
            lightEnvironment.dirLightIntensity = dirLightComponentIt->getIntensity();
            lightEnvironment.dirLightCastShadows = dirLightComponentIt->getCastShadows();
        }

        for(auto it = componentManager.cbegin<PointLightComponent>(); it != componentManager.cend<PointLightComponent>(); it++) {
            ScenePointLight pointLight{};
            pointLight.position = componentManager.getComponent<TransformComponent>(it->getEntity()).getGlobalPosition();
            pointLight.color = it->getColor();
            pointLight.falloff = it->getFalloff();
            pointLight.radius = it->getRadius();
            pointLight.intensity = it->getIntensity();
            lightEnvironment.pointLights.push_back(pointLight);
        }

        for(auto it = componentManager.cbegin<SpotLightComponent>(); it != componentManager.cend<SpotLightComponent>(); it++) {
            SceneSpotLight spotLight{};
            spotLight.position = componentManager.getComponent<TransformComponent>(it->getEntity()).getGlobalPosition();
            spotLight.direction = glm::normalize(glm::mat3(componentManager.getComponent<TransformComponent>(it->getEntity()).getModelMatrix()) * glm::vec3(0.0f, -1.0f, 0.0f));
            spotLight.color = it->getColor();
            spotLight.falloff = it->getFalloff();
            spotLight.radius = it->getRadius();
            spotLight.intensity = it->getIntensity();
            spotLight.innerAngle = it->getInnerAngle();
            spotLight.outerAngle = it->getOuterAngle();
            lightEnvironment.spotLights.push_back(spotLight);
        }

        SceneEnvironment sceneEnvironment{};

        auto skyboxComponentIt = componentManager.cbegin<SkyboxComponent>();
        if (skyboxComponentIt != componentManager.cend<SkyboxComponent>()) {
            sceneEnvironment.environmentMapDescriptorSet = skyboxComponentIt->getDescriptorSet();
            sceneEnvironment.environmentIntensity = skyboxComponentIt->getIntensity();
            sceneEnvironment.environmentLod = skyboxComponentIt->getLod();
        } else {
            sceneEnvironment.environmentMapDescriptorSet = nullptr;
            sceneEnvironment.environmentIntensity = 0.0f;
            sceneEnvironment.environmentLod = 1.0f;
        }

        renderer.setActiveScene(this);
        auto cameraTransformWithoutScale = glm::translate(glm::mat4(1.0f), cameraTransform.getGlobalPosition()) * glm::toMat4(cameraTransform.getGlobalRotationQuat());
        renderer.beginScene(cameraComponent.getCamera(), cameraTransformWithoutScale, lightEnvironment, sceneEnvironment);

        auto& applicationOptions = Application::get().getApplicationOptions();

        for (auto it = componentManager.begin<MeshRendererComponent>(); it != componentManager.end<MeshRendererComponent>(); it++) {
            if (it->isActive()) {
                bool hasLodDistanceComponent = componentManager.hasComponent<LodDistanceComponent>(it->getEntity());
                renderer.submitMesh(it->getRenderMesh(), it->getMeshNodes(), it->getMaterial().get(), it->getCastShadows(), it->getCullingEnabled(), componentManager.getComponent<TransformComponent>(it->getEntity()).getModelMatrix(), hasLodDistanceComponent ? componentManager.getComponent<LodDistanceComponent>(it->getEntity()).getLodDistances() : applicationOptions.defaultLodDistances);
            }
        }

        for (auto it = componentManager.begin<AnimatedMeshRendererComponent>(); it != componentManager.end<AnimatedMeshRendererComponent>(); it++) {
            if (it->isActive()) renderer.submitAnimatedMesh(it->getMeshVertices().get(), it->getMeshNodes(), it->getMaterial().get(), it->getCastShadows(), componentManager.getComponent<TransformComponent>(it->getEntity()).getModelMatrix(), it->getBoneTransforms(), it->getSkinnedVAO(), it->getSkinningDescriptorSet());
        }

        for (auto it = componentManager.begin<CustomShaderRendererComponent>(); it != componentManager.end<CustomShaderRendererComponent>(); it++) {
            if (it->isActive()) {
                bool hasLodDistanceComponent = componentManager.hasComponent<LodDistanceComponent>(it->getEntity());
                renderer.submitCustomShaderMesh(it->getRenderMesh(), it->getMeshNodes(), it->getCullingEnabled(), it->getBoundingBox(), componentManager.getComponent<TransformComponent>(it->getEntity()).getModelMatrix(), it->getPipeline().get(), it->getInstanceCount(), hasLodDistanceComponent ? componentManager.getComponent<LodDistanceComponent>(it->getEntity()).getLodDistances() : applicationOptions.defaultLodDistances, it->getDescriptorSet());
            }
        }

        for (auto it = componentManager.cbegin<UiCanvasComponent2D>(); it != componentManager.cend<UiCanvasComponent2D>(); it++) {
            renderer.submitUiCanvas2D(it->getCanvas(), it->getFinalTransform(), it->getZIndex());
        }

        executeOnRenderFunctions(renderer);

        #ifdef CG_ENABLE_DEBUG_FEATURES
        if (applicationOptions.debugShowPhysicsColliders) {
            auto& resourceManager = Application::get().getResourceManager();

            auto* cubeMesh = resourceManager.getResource<MeshVertices>("CG_CubeMesh").get();
            for (auto it = componentManager.begin<BoxColliderComponent>(); it != componentManager.end<BoxColliderComponent>(); it++) {
                auto modelMatrix = componentManager.getComponent<TransformComponent>(it->getEntity()).getModelMatrix();
                glm::mat4 colliderTransform = glm::translate(glm::mat4(1.0), it->getOffset()) * modelMatrix * glm::scale(glm::mat4(1.0f), it->getHalfSize() * 2.0f);
                renderer.submitPhysicsColliderMesh(cubeMesh, colliderTransform);
            }

            auto* sphereMesh = resourceManager.getResource<MeshVertices>("CG_SphereMesh_16_16").get();
            for (auto it = componentManager.begin<SphereColliderComponent>(); it != componentManager.end<SphereColliderComponent>(); it++) {
                auto modelMatrix = componentManager.getComponent<TransformComponent>(it->getEntity()).getModelMatrix();
                glm::mat4 colliderTransform = glm::translate(glm::mat4(1.0), it->getOffset()) * modelMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(it->getRadius()));
                renderer.submitPhysicsColliderMesh(sphereMesh, colliderTransform);
            }

            for (auto it = componentManager.begin<CapsuleColliderComponent>(); it != componentManager.end<CapsuleColliderComponent>(); it++) {
                auto& transform = componentManager.getComponent<TransformComponent>(it->getEntity());
                float radius = it->getRadius() * glm::max(transform.getGlobalScale().x, transform.getGlobalScale().z);
                auto* capsuleMesh = resourceManager.getResource<MeshVertices>("CG_CapsuleMesh_" + std::to_string(radius) + "_" + std::to_string(it->getHalfHeight() * 2.0f * transform.getGlobalScale().y)).get();
                glm::mat4 colliderTransform = glm::translate(glm::mat4(1.0), transform.getGlobalPosition() + it->getOffset());
                renderer.submitPhysicsColliderMesh(capsuleMesh, colliderTransform);
            }

            for (auto it = componentManager.begin<TriangleColliderComponent>(); it != componentManager.end<TriangleColliderComponent>(); it++) {
                auto& transform = componentManager.getComponent<TransformComponent>(it->getEntity());
                renderer.submitPhysicsColliderMesh(it->getPhysicsMesh().getVisualizationMesh(), glm::translate(glm::scale(glm::mat4(1.0), transform.getGlobalScale()), transform.getGlobalPosition()));
            }

            for (auto it = componentManager.begin<ConvexColliderComponent>(); it != componentManager.end<ConvexColliderComponent>(); it++) {
                auto& transform = componentManager.getComponent<TransformComponent>(it->getEntity());
                renderer.submitPhysicsColliderMesh(it->getPhysicsMesh().getVisualizationMesh(), glm::scale(glm::translate(glm::mat4(1.0f), transform.getGlobalPosition()), transform.getGlobalScale()));
            }
        }

        if (applicationOptions.debugShowBoundingBoxes) {
            auto& resourceManager = Application::get().getResourceManager();

            auto* cubeMesh = resourceManager.getResource<MeshVertices>("CG_CubeMesh").get();
            for (auto it = componentManager.begin<MeshRendererComponent>(); it != componentManager.end<MeshRendererComponent>(); it++) {
                if (it->isActive()) {
                    auto& transform = componentManager.getComponent<TransformComponent>(it->getEntity());
                    renderer.submitBoundingBoxMesh(cubeMesh, it->getRenderMesh(), it->getMeshNodes(), transform.getModelMatrix());
                }
            }
            for (auto it = componentManager.begin<CustomShaderRendererComponent>(); it != componentManager.end<CustomShaderRendererComponent>(); it++) {
                if (it->isActive()) {
                    auto& transform = componentManager.getComponent<TransformComponent>(it->getEntity());
                    if (it->getBoundingBox() != nullptr) {
                        renderer.submitBoundingBoxMesh(cubeMesh, *it->getBoundingBox(), transform.getModelMatrix());
                    } else {
                        renderer.submitBoundingBoxMesh(cubeMesh, it->getRenderMesh(), it->getMeshNodes(), transform.getModelMatrix());
                    }
                }
            }
        }
        #endif

        renderer.endScene();
    }

    void Scene::executeFixedUpdate(TimeStep ts) {
        for (auto it = componentManager.begin<ScriptComponent>(); it != componentManager.end<ScriptComponent>(); it++) {
            it->fixedUpdate(ts);
        }
        executeAllPendingOperations(false);
    }

    void Scene::executeAllPendingOperations(bool forceUpdateTransforms) {
        std::unordered_set<Entity> recursivelyDestroyedEntities{};

        for (const auto& entity: entitiesToBeDestroyed) {
            recursivelyDestroyedEntities.insert(entity);
            findRecursiveEntitiesToDestroy(entity, recursivelyDestroyedEntities);
        }

        for (const auto& entity : recursivelyDestroyedEntities) {
            componentManager.destroyEntity(entity, *this);
        }

        for (const auto& entity: entitiesToBeDestroyed) {
            if (parents.find(entity) != parents.end()) {
                children[parents[entity]].erase(entity);
            }
        }
        entitiesToBeDestroyed.clear();

        bool wereComponentsAdded = componentManager.executePendingOperations(*this);

        for (const auto& entity: recursivelyDestroyedEntities) {
            std::optional<std::string> entityId = getIdForEntity(entity);
            if (entityId.has_value()) {
                idToEntity.erase(entityId->erase());
            }

            entityTags.erase(entity);
            parents.erase(entity);
            children.erase(entity);
        }


        if (wereComponentsAdded || forceUpdateTransforms) {
            updateTransforms();
        }

        componentManager.callOnEnableForAddedComponents(*this);
    }

    int Scene::getViewportWidth() const {
        return viewportWidth;
    }

    int Scene::getViewportHeight() const {
        return viewportHeight;
    }

    CameraComponent& Scene::getPrimaryCamaraComponent() {
        auto cameraComponent = std::find_if(componentManager.begin<CameraComponent>(), componentManager.end<CameraComponent>(), [](auto&& c) { return c.isPrimary();});
        CG_ASSERT(cameraComponent != componentManager.end<CameraComponent>(), "Scene must have a Primary Camera")

        return *cameraComponent;
    }

    ComponentHandle<CameraComponent> Scene::getPrimaryCameraComponentHandle() {
        const auto& primaryCameraComponent = getPrimaryCamaraComponent();
        return {&componentManager, primaryCameraComponent.getEntity()};
    }

    void Scene::findRecursiveEntitiesToDestroy(Entity entity, std::unordered_set<Entity>& recursivelyDestroyedEntities) {
        for (const auto &child : children[entity]) {
            recursivelyDestroyedEntities.insert(child);
            findRecursiveEntitiesToDestroy(child, recursivelyDestroyedEntities);
        }
    }

    void Scene::recursiveUpdateChildTransforms(Entity entity, const glm::mat4& parentModelMatrix, bool parentDirty) {
        auto& transform = componentManager.getComponent<TransformComponent>(entity);
        bool dirty = transform._calculateChildTransformsWithParent(parentModelMatrix, parentDirty);

        for (const auto &child: children[entity]) {
            recursiveUpdateChildTransforms(child, transform.getModelMatrix(), dirty);
        }
    }

    void Scene::executeOnRenderFunctions(SceneRenderer& renderer) {
        for (const auto& function : onRenderFunctions) {
            function(renderer);
        }
        onRenderFunctions.clear();
    }
}
