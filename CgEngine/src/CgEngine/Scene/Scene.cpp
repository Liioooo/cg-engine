#include "Scene.h"
#include "Asserts.h"
#include "Rendering/SceneRenderer.h"
#include "Application.h"
#include "Audio/AudioComponentUpdateData.h"

namespace CgEngine {
    Scene::Scene(int viewportWidth, int viewportHeight) : viewportWidth(viewportWidth), viewportHeight(viewportHeight) {
        physicsScene = new PhysicsScene();
    }

    Scene::~Scene() {
        componentManager->destroyAllComponents(*this);
        delete componentManager;
        delete physicsScene;
    }

    Entity Scene::createEntity(Entity parent) {
        if (parent == NoEntity) {
            entityCount++;
            Entity entity = nextEntityId++;
            children[entity] = std::unordered_set<Entity>();
            return entity;
        } else if (children.find(parent) == children.end()) {
            CG_ASSERT(false, "Entity parent doesn't exist")
            return NoEntity;
        } else {
            entityCount++;
            Entity entity = nextEntityId++;
            children[parent].insert(entity);
            children[entity] = std::unordered_set<Entity>();
            parents[entity] = parent;
            return entity;
        }
    }

    Entity Scene::createEntity(Entity parent, const std::string& id) {
        if (idToEntity.find(id) != idToEntity.end()) {
            CG_ASSERT(false, "Entity-Id is already used")
            return NoEntity;
        }

        if (parent == NoEntity) {
            entityCount++;
            Entity entity = nextEntityId++;
            idToEntity[id] = entity;
            children[entity] = std::unordered_set<Entity>();
            return entity;
        } else if (children.find(parent) == children.end()) {
            CG_ASSERT(false, "Entity parent doesn't exist")
            return NoEntity;
        } else {
            entityCount++;
            Entity entity = nextEntityId++;
            idToEntity[id] = entity;
            children[parent].insert(entity);
            children[entity] = std::unordered_set<Entity>();
            parents[entity] = parent;
            return entity;
        }
    }

    void Scene::destroyEntity(Entity entity) {
        if (parents.find(entity) != parents.end()) {
            children[parents[entity]].erase(entity);
        }
        recursiveDestroyEntity(entity);
    }

    Entity Scene::findEntityById(const std::string &id) {
        if (idToEntity.find(id) == idToEntity.end()) {
            CG_LOGGING_WARNING("Entity {0} doesn't exist!", id)
            return NoEntity;
        }
        return idToEntity[id];
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

    const std::unordered_set<Entity>& Scene::getChildren(Entity entity) {
        return children[entity];
    }

    Entity Scene::getParent(Entity entity) {
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

    void Scene::setEntityTag(CgEngine::Entity entity, const std::string& tag) {
        if (hasEntity(entity)) {
            entityTags[entity] = tag;
        }
    }

    std::string Scene::getEntityTag(CgEngine::Entity entity) {
        if (entityTags.find(entity) != entityTags.end()) {
            return entityTags.at(entity);
        }
        return "";
    }

    void Scene::updateTransforms() {
        for (const auto &[entity, _] : children) {
            if (parents.find(entity) == parents.end()) {
                auto& topLevelTransform = componentManager->getComponent<TransformComponent>(entity);
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

        auto cameras = componentManager->getEntitiesWithComponent<CameraComponent>();
        for (const auto &item: cameras) {
            auto& camaraComp = getComponent<CameraComponent>(item);
            camaraComp.getCamera().setViewportSize(width, height);
        }
    }

    void Scene::submitPostUpdateFunction(std::function<void()>&& function) {
        postUpdateFunctions.emplace_back(function);
    }

    Uuid Scene::submitOnPreRenderFunction(const std::function<void(const CameraFrustum& camaraFrustum)>& function, bool once) {
        return onPreRenderFunctions.emplace_back(function, once).uuid;
    }

    void Scene::removeOnPreRenderFunction(Uuid uuid) {
        onPreRenderFunctions.erase(std::remove_if(onPreRenderFunctions.begin(), onPreRenderFunctions.end(), [uuid](const auto& fc) {
            return uuid == fc.uuid;
        }), onPreRenderFunctions.end());
    }

    Uuid Scene::submitOnRenderFunction(const std::function<void(SceneRenderer&)>& function, bool once) {
        return onRenderFunctions.emplace_back(function, once).uuid;
    }

    void Scene::removeOnRenderFunction(Uuid uuid) {
        onRenderFunctions.erase(std::remove_if(onRenderFunctions.begin(), onRenderFunctions.end(), [uuid](const auto& fc) {
            return uuid == fc.uuid;
        }), onRenderFunctions.end());
    }

    void Scene::onUpdate(TimeStep ts) {
        physicsScene->simulate(ts, *this);

        for (auto it = componentManager->begin<ScriptComponent>(); it != componentManager->end<ScriptComponent>(); it++) {
            it->update(ts);
        }
        executePostUpdateFunctions();

        for (auto it = componentManager->begin<AnimationComponent>(); it != componentManager->end<AnimationComponent>(); it++) {
            it->update(ts, componentManager->getComponent<TransformComponent>(it->getEntity()));
        }

        updateTransforms();

        for (auto it = componentManager->begin<ScriptComponent>(); it != componentManager->end<ScriptComponent>(); it++) {
            it->lateUpdate(ts);
        }
        executePostUpdateFunctions();

        updateTransforms();

        for (auto it = componentManager->begin<UiCanvasComponent>(); it != componentManager->end<UiCanvasComponent>(); it++) {
            it->update(viewportWidth, viewportHeight);
        }

        for (auto it = componentManager->begin<AnimatedMeshRendererComponent>(); it != componentManager->end<AnimatedMeshRendererComponent>(); it++) {
            if (it->isActive()) it->update(ts);
        }

        if (!componentManager->getEntitiesWithComponent<AudioListenerComponent>().empty()) {
            for (auto it = componentManager->begin<AudioListenerComponent>(); it != componentManager->end<AudioListenerComponent>(); it++) {
                if (it->isActive()) {
                    auto& audioSystem = AudioSystem::get();

                    auto& transform = componentManager->getComponent<TransformComponent>(it->getEntity());
                    audioSystem.updateListenerPosition({transform.getGlobalRotationQuat(), transform.getGlobalPosition()});
                    audioSystem.updateListenerVolume(it->getVolume());
                    if (componentManager->hasComponent<RigidBodyComponent>(it->getEntity())) {
                        auto& rigidBody = componentManager->getComponent<RigidBodyComponent>(it->getEntity());
                        if (rigidBody.isDynamic()) {
                            audioSystem.updateListenerVelocity(rigidBody.getLinearVelocity());
                        }
                    }
                    break;
                }
            }
        } else {
            auto& primaryCamera = getPrimaryCamaraComponent();
            auto& audioSystem = AudioSystem::get();

            auto& transform = componentManager->getComponent<TransformComponent>(primaryCamera.getEntity());
            audioSystem.updateListenerPosition({transform.getGlobalRotationQuat(), transform.getGlobalPosition()});
            audioSystem.updateListenerVolume(1.0f);
            if (componentManager->hasComponent<RigidBodyComponent>(primaryCamera.getEntity())) {
                auto& rigidBody = componentManager->getComponent<RigidBodyComponent>(primaryCamera.getEntity());
                if (rigidBody.isDynamic()) {
                    audioSystem.updateListenerVelocity(rigidBody.getLinearVelocity());
                }
            }
        }

        const auto audioComponentsMarkedForDestroy = AudioSystem::get().getComponentsMarkedForDestroy();
        std::vector<Entity> audioComponentsToDestroy;
        audioComponentsToDestroy.reserve(audioComponentsMarkedForDestroy.size());

        std::vector<AudioComponentUpdateData> audioComponentUpdateData;
        for (auto it = componentManager->begin<AudioComponent>(); it != componentManager->end<AudioComponent>(); it++) {
            if (audioComponentsMarkedForDestroy.find(it->getUuid()) != audioComponentsMarkedForDestroy.end()) {
                audioComponentsToDestroy.emplace_back(it->getEntity());
                continue;
            }

            auto& transform = componentManager->getComponent<TransformComponent>(it->getEntity());

            auto& updateData = audioComponentUpdateData.emplace_back();
            updateData.uuid = it->getUuid();
            updateData.looping = it->isLooping();
            updateData.volume = it->getVolume();
            updateData.pitch = it->getPitch();
            updateData.transform = {transform.getGlobalRotationQuat(), transform.getGlobalPosition()};

            if (componentManager->hasComponent<RigidBodyComponent>(it->getEntity())) {
                auto& rigidBody = componentManager->getComponent<RigidBodyComponent>(it->getEntity());
                if (rigidBody.isDynamic()) {
                    updateData.velocity = rigidBody.getLinearVelocity();
                }
            }
        }
        AudioSystem::get().updateAudioComponents(std::move(audioComponentUpdateData));

        for (const auto& entity: audioComponentsToDestroy) {
            detachComponent<AudioComponent>(entity);
        }
    }

    void Scene::onEvent(Event& event) {
        for (auto it = componentManager->begin<ScriptComponent>(); it != componentManager->end<ScriptComponent>(); it++) {
            it->onEvent(event);
            if (event.wasHandled()) {
                return;
            }
        }
        executePostUpdateFunctions();
    }

    void Scene::onRender(SceneRenderer& renderer) {
        auto& cameraComponent = getPrimaryCamaraComponent();

        auto cameraTransform = componentManager->getComponent<TransformComponent>(cameraComponent.getEntity());

        SceneLightEnvironment lightEnvironment{};

        auto dirLightComponentIt = componentManager->cbegin<DirectionalLightComponent>();
        if (dirLightComponentIt != componentManager->cend<DirectionalLightComponent>()) {
            lightEnvironment.dirLightDirection = glm::normalize(glm::mat3(componentManager->getComponent<TransformComponent>(dirLightComponentIt->getEntity()).getModelMatrix()) * glm::vec3(0.0f, 1.0f, 0.0f));
            lightEnvironment.dirLightColor = dirLightComponentIt->getColor();
            lightEnvironment.dirLightIntensity = dirLightComponentIt->getIntensity();
            lightEnvironment.dirLightCastShadows = dirLightComponentIt->getCastShadows();
        }

        for(auto it = componentManager->cbegin<PointLightComponent>(); it != componentManager->cend<PointLightComponent>(); it++) {
            ScenePointLight pointLight{};
            pointLight.position = componentManager->getComponent<TransformComponent>(it->getEntity()).getGlobalPosition();
            pointLight.color = it->getColor();
            pointLight.falloff = it->getFalloff();
            pointLight.radius = it->getRadius();
            pointLight.intensity = it->getIntensity();
            lightEnvironment.pointLights.push_back(pointLight);
        }

        for(auto it = componentManager->cbegin<SpotLightComponent>(); it != componentManager->cend<SpotLightComponent>(); it++) {
            SceneSpotLight spotLight{};
            spotLight.position = componentManager->getComponent<TransformComponent>(it->getEntity()).getGlobalPosition();
            spotLight.direction = glm::normalize(glm::mat3(componentManager->getComponent<TransformComponent>(it->getEntity()).getModelMatrix()) * glm::vec3(0.0f, -1.0f, 0.0f));
            spotLight.color = it->getColor();
            spotLight.falloff = it->getFalloff();
            spotLight.radius = it->getRadius();
            spotLight.intensity = it->getIntensity();
            spotLight.innerAngle = it->getInnerAngle();
            spotLight.outerAngle = it->getOuterAngle();
            lightEnvironment.spotLights.push_back(spotLight);
        }

        SceneEnvironment sceneEnvironment{};

        auto skyboxComponentIt = componentManager->cbegin<SkyboxComponent>();
        if (skyboxComponentIt != componentManager->cend<SkyboxComponent>()) {
            sceneEnvironment.irradianceMap = skyboxComponentIt->getIrradianceMap().get();
            sceneEnvironment.prefilterMap = skyboxComponentIt->getPrefilterMap().get();
            sceneEnvironment.environmentIntensity = skyboxComponentIt->getIntensity();
            sceneEnvironment.environmentLod = skyboxComponentIt->getLod();
        } else {
            sceneEnvironment.irradianceMap = &Renderer::getBlackCubeTexture();
            sceneEnvironment.prefilterMap = &Renderer::getBlackCubeTexture();
            sceneEnvironment.environmentIntensity = 0.0f;
            sceneEnvironment.environmentLod = 1.0f;
        }

        renderer.setActiveScene(this);
        renderer.beginScene(cameraComponent.getCamera(), cameraTransform.getModelMatrix(), lightEnvironment, sceneEnvironment);

        executeOnPreRenderFunctions(renderer);

        for (auto it = componentManager->begin<MeshRendererComponent>(); it != componentManager->end<MeshRendererComponent>(); it++) {
            if (it->isActive()) renderer.submitMesh(it->getRenderMesh(), it->getMeshNodes(), it->getMaterial().get(), it->getCastShadows(), it->getCullingEnabled(), componentManager->getComponent<TransformComponent>(it->getEntity()).getModelMatrix());
        }

        for (auto it = componentManager->begin<AnimatedMeshRendererComponent>(); it != componentManager->end<AnimatedMeshRendererComponent>(); it++) {
            if (it->isActive()) renderer.submitAnimatedMesh(it->getMeshVertices().get(), it->getMeshNodes(), it->getMaterial().get(), it->getCastShadows(), componentManager->getComponent<TransformComponent>(it->getEntity()).getModelMatrix(), it->getBoneTransforms(), it->getSkinnedVAO());
        }

        for (auto it = componentManager->begin<CustomShaderRendererComponent>(); it != componentManager->end<CustomShaderRendererComponent>(); it++) {
            if (it->isActive()) renderer.submitCustomShaderMesh(it->getRenderMesh(), it->getMeshNodes(), it->getRenderMaterial(), it->getCullingEnabled(), it->getBoundingBox(), componentManager->getComponent<TransformComponent>(it->getEntity()).getModelMatrix(), it->getShader().get(), it->getInstanceCount(), it->getRenderPassOptions(), it->getInstanceBuffers());
        }

        for (auto it = componentManager->cbegin<UiCanvasComponent>(); it != componentManager->cend<UiCanvasComponent>(); it++) {
            renderer.submitUiElements(it->getUiElements());
        }

        executeOnRenderFunctions(renderer);

#ifdef CG_ENABLE_DEBUG_FEATURES
        auto& applicationOptions = Application::get().getApplicationOptions();
        if (applicationOptions.debugShowPhysicsColliders) {
            auto& resourceManager = Application::get().getResourceManager();

            auto* cubeMesh = resourceManager.getResource<MeshVertices>("CG_CubeMesh").get();
            for (auto it = componentManager->begin<BoxColliderComponent>(); it != componentManager->end<BoxColliderComponent>(); it++) {
                auto modelMatrix = componentManager->getComponent<TransformComponent>(it->getEntity()).getModelMatrix();
                glm::mat4 colliderTransform = glm::translate(glm::mat4(1.0), it->getOffset()) * modelMatrix * glm::scale(glm::mat4(1.0f), it->getHalfSize() * 2.0f);
                renderer.submitPhysicsColliderMesh(cubeMesh, colliderTransform);
            }

            auto* sphereMesh = resourceManager.getResource<MeshVertices>("CG_SphereMesh_16_16").get();
            for (auto it = componentManager->begin<SphereColliderComponent>(); it != componentManager->end<SphereColliderComponent>(); it++) {
                auto modelMatrix = componentManager->getComponent<TransformComponent>(it->getEntity()).getModelMatrix();
                glm::mat4 colliderTransform = glm::translate(glm::mat4(1.0), it->getOffset()) * modelMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(it->getRadius()));
                renderer.submitPhysicsColliderMesh(sphereMesh, colliderTransform);
            }

            for (auto it = componentManager->begin<CapsuleColliderComponent>(); it != componentManager->end<CapsuleColliderComponent>(); it++) {
                auto& transform = componentManager->getComponent<TransformComponent>(it->getEntity());
                float radius = it->getRadius() * glm::max(transform.getGlobalScale().x, transform.getGlobalScale().z);
                auto* capsuleMesh = resourceManager.getResource<MeshVertices>("CG_CapsuleMesh_" + std::to_string(radius) + "_" + std::to_string(it->getHalfHeight() * 2.0f * transform.getGlobalScale().y)).get();
                glm::mat4 colliderTransform = glm::translate(glm::mat4(1.0), transform.getGlobalPosition() + it->getOffset());
                renderer.submitPhysicsColliderMesh(capsuleMesh, colliderTransform);
            }

            for (auto it = componentManager->begin<TriangleColliderComponent>(); it != componentManager->end<TriangleColliderComponent>(); it++) {
                auto& transform = componentManager->getComponent<TransformComponent>(it->getEntity());
                renderer.submitPhysicsColliderMesh(it->getPhysicsMesh().getVisualizationMesh(), glm::translate(glm::scale(glm::mat4(1.0), transform.getGlobalScale()), transform.getGlobalPosition()));
            }

            for (auto it = componentManager->begin<ConvexColliderComponent>(); it != componentManager->end<ConvexColliderComponent>(); it++) {
                auto& transform = componentManager->getComponent<TransformComponent>(it->getEntity());
                renderer.submitPhysicsColliderMesh(it->getPhysicsMesh().getVisualizationMesh(), glm::scale(glm::translate(glm::mat4(1.0f), transform.getGlobalPosition()), transform.getGlobalScale()));
            }
        }

        if (applicationOptions.debugShowBoundingBoxes) {
            auto& resourceManager = Application::get().getResourceManager();

            auto* cubeMesh = resourceManager.getResource<MeshVertices>("CG_CubeMesh").get();
            for (auto it = componentManager->begin<MeshRendererComponent>(); it != componentManager->end<MeshRendererComponent>(); it++) {
                auto& transform = componentManager->getComponent<TransformComponent>(it->getEntity());
                renderer.submitBoundingBoxMesh(cubeMesh, it->getRenderMesh(), it->getMeshNodes(), transform.getModelMatrix());
            }
            for (auto it = componentManager->begin<CustomShaderRendererComponent>(); it != componentManager->end<CustomShaderRendererComponent>(); it++) {
                auto& transform = componentManager->getComponent<TransformComponent>(it->getEntity());
                renderer.submitBoundingBoxMesh(cubeMesh, it->getBoundingBox(), transform.getModelMatrix());
            }
        }
#endif

        renderer.endScene();
    }

    void Scene::executeFixedUpdate(TimeStep ts) {
        for (auto it = componentManager->begin<ScriptComponent>(); it != componentManager->end<ScriptComponent>(); it++) {
            it->fixedUpdate(ts);
        }
        executePostUpdateFunctions();
    }

    int Scene::getViewportWidth() const {
        return viewportWidth;
    }

    int Scene::getViewportHeight() const {
        return viewportHeight;
    }

    CameraComponent& Scene::getPrimaryCamaraComponent() {
        auto cameraComponent = std::find_if(componentManager->begin<CameraComponent>(), componentManager->end<CameraComponent>(), [](auto&& c) { return c.isPrimary();});
        CG_ASSERT(cameraComponent != componentManager->end<CameraComponent>(), "Scene must have a Primary Camera")

        return *cameraComponent;
    }

    void Scene::recursiveDestroyEntity(Entity entity) {
        for (const auto &child : children[entity]) {
            recursiveDestroyEntity(child);
        }

        std::optional<std::string> entityId = getIdForEntity(entity);
        if (entityId.has_value()) {
            idToEntity.erase(entityId->erase());
        }

        entityTags.erase(entity);

        componentManager->destroyEntity(entity, *this);
        parents.erase(entity);
        children.erase(entity);
        entityCount--;
    }

    void Scene::recursiveUpdateChildTransforms(Entity entity, const glm::mat4& parentModelMatrix, bool parentDirty) {
        auto& transform = componentManager->getComponent<TransformComponent>(entity);
        bool dirty = transform._calculateChildTransformsWithParent(parentModelMatrix, parentDirty);

        for (const auto &child: children[entity]) {
            recursiveUpdateChildTransforms(child, transform.getModelMatrix(), dirty);
        }
    }

    void Scene::executePostUpdateFunctions() {
        for (const auto &fn: postUpdateFunctions) {
            fn();
        }
        postUpdateFunctions.clear();
    }

    void Scene::executeOnPreRenderFunctions(SceneRenderer& renderer) {
        for (auto it = onPreRenderFunctions.begin(); it != onPreRenderFunctions.end();) {
            it->function(renderer.getCamaraFrustum());

            if (it->once) {
                it = onPreRenderFunctions.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Scene::executeOnRenderFunctions(SceneRenderer& renderer) {
        for (auto it = onRenderFunctions.begin(); it != onRenderFunctions.end();) {
            it->function(renderer);

            if (it->once) {
                it = onRenderFunctions.erase(it);
            } else {
                ++it;
            }
        }
    }
}
