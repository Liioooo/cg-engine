#include "ComponentManager.h"
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

    ComponentManager::ComponentManager() {
        registerComponentType<TransformComponent>();
        registerComponentType<ScriptComponent>();
        registerComponentType<MeshRendererComponent>();
        registerComponentType<AnimatedMeshRendererComponent>();
        registerComponentType<CameraComponent>();
        registerComponentType<DirectionalLightComponent>();
        registerComponentType<PointLightComponent>();
        registerComponentType<SpotLightComponent>();
        registerComponentType<SkyboxComponent>();
        registerComponentType<BoxColliderComponent>();
        registerComponentType<SphereColliderComponent>();
        registerComponentType<CapsuleColliderComponent>();
        registerComponentType<TriangleColliderComponent>();
        registerComponentType<ConvexColliderComponent>();
        registerComponentType<RigidBodyComponent>();
        registerComponentType<CharacterControllerComponent>();
        registerComponentType<CharacterControllerComponent>();
        registerComponentType<UiCanvasComponent2D>();
        registerComponentType<AnimationComponent>();
        registerComponentType<CustomShaderRendererComponent>();
        registerComponentType<AudioListenerComponent>();
        registerComponentType<AudioComponent>();
        registerComponentType<LodDistanceComponent>();
    }

}
