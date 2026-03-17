# Available Components

## TransformComponent

| **Property** | **Description**                    | **Example** | **Default** |
|--------------|------------------------------------|-------------|-------------|
| `position`   | Translation of the entity          | `0 0 0`     | `0 0 0`     |
| `rotation`   | Local rotation of the entity (deg) | `0 0 0`     | `0 0 0`     |
| `scale`      | Local scale of the entity          | `0 0 0`     | `1 1 1`     |

## MeshRendererComponent

| **Property**     | **Description**                                                 | **Example**           | **Default** |
|------------------|-----------------------------------------------------------------|-----------------------|-------------|
| `asset-file`     | File to load 3D-Asset                                           | `model.fbx`           | -           |
| `mesh`           | Built in mesh                                                   | `CG_SphereMesh_16_16` | -           |
| `customMesh`     | Instance of `CgEngine::CustomMesh*` (can only be set from code) | -                     | -           |
| `material`       | Material for this mesh. Overrides materials from asset files.   | `MyMaterial`          | -           |
| `cast-shadows`   | Defines if this object casts shadows.                           | `true`                | `true`      |
| `enable-culling` | Defines if this object is culled using it's AABoundingBoxes.    | `true`                | `true`      |
| `mesh-nodes`     | List of Nodes to render. Nothing means: everything is rendered. | `RootNode, Node1`     | -           |

**Built in Meshes:**

- CG_CubeMesh
- CG_SphereMesh
  - `CG_SphereMesh_N_M`: <br> 
    `N`: number of longitudinal Segments <br>
    `M`: number of latitudinal Segments
- CG_Capsule
  - `CG_Capsule_R_H`: <br>
    `R`: Radius <br>
    `H`: Height

## AnimatedMeshRendererComponent

| **Property**           | **Description**                                                 | **Example**           | **Default** |
|------------------------|-----------------------------------------------------------------|-----------------------|-------------|
| `asset-file`           | File to load 3D-Asset                                           | `model.fbx`           | -           |
| `material`             | Material for this mesh. Overrides materials from asset files.   | `MyMaterial`          | -           |
| `cast-shadows`         | Defines if this object casts shadows.                           | `true`                | `true`      |
| `mesh-nodes`           | List of Nodes to render. Nothing means: everything is rendered. | `RootNode, Node1`     | -           |
| `animation`            | Animation to use. (Must be included in the Asset-File)          | `Armature\|Animation` | -           |
| `animation-speed`      | Animation speed. (Can also be negative)                         | `1.0`                 | `1.0`       |
| `animation-start-time` | Animation start time                                            | `0.5`                 | `0.0`       |
| `auto-play`            | Auto play the animation.                                        | `true`                | `true`      |
| `loop`                 | Should the animation loop?                                      | `true`                | `true`      |

## CustomShaderRendererComponent
| **Property**                   | **Description**                                                          | **Example**           | **Default** |
|--------------------------------|--------------------------------------------------------------------------|-----------------------|-------------|
| `asset-file`                   | File to load 3D-Asset                                                    | `model.fbx`           | -           |
| `mesh`                         | Built in mesh                                                            | `CG_SphereMesh_16_16` | -           |
| `material`                     | Material for this mesh. Overrides materials from asset files.            | `MyMaterial`          | -           |
| `enable-culling`               | Defines if this object is culled using it's AABoundingBoxes.             | `true`                | `true`      |
| `bouding-min`                  | Defines the max extent of a AABB.                                        | `-1 -1 -1`            | `0 0 0`     |
| `bounding-max`                 | Defines the min extent of a AABB.                                        | `1 1 1`               | `0 0 0`     |
| `mesh-nodes`                   | List of Nodes to render. Nothing means: everything is rendered.          | `RootNode, Node1`     | -           |
| `instance-count`               | Amount of instances that will be rendered.                               | `10`                  | `1`         |
| `shader`                       | Shader that will be used to render.                                      | `MyShader`            | -           |
| `use-environment-mapping-data` | Specify if, environment mapping data should be bound to the shader.      | `true`                | `false`     |
| `use-dir-shadow-mapping-data`  | Specify if, direction shadow mapping data should be bound to the shader. | `true`                | `false`     |
| `customMesh`                   | Instance of `CgEngine::CustomMesh*` (can only be set from code)          | -                     | -           |

Additional properties can be set in scripts using `CgEngine::CustomShaderRendererComponentParams`.

It is also possible to set up to 2 `instanceBuffers`. This buffers will be bound to binding points `5` and `6` respectively, in the shader.

When using `use-environment-mapping-data` or `use-dir-shadow-mapping-data` the shader can use `#include "common/IBLCalculationsFragment.glsl"` and `#include "common/DirShadowMapping{Vertex|Fragment}.glsl"` to make use of this data.

When defining a custom shader in the `assets/shaders.xml` file, the `forward` attribute can be set to `true` to use the forward rendering pipeline. To use the deferred rendering pipeline, the `forward` attribute should be set to `false`.

Generally everything located at `CgEngine/assets/shaders/common` can be imported into custom shaders to make use of the functionality exposed there.

## CameraComponent

| **Property**      | **Description**                                                              | **Example**   | **Default**   |
|-------------------|------------------------------------------------------------------------------|---------------|---------------|
| `projection`      | Projection: `perspective` or `orthogonal`                                    | `perspective` | `perspective` |
| `near`            | Near plane                                                                   | `0.1`         | `0.1`         |
| `far`             | Far plane                                                                    | `100`         | `100`         |
| `fov`             | Field of View (deg)                                                          | `60`          | `60`          |
| `ortho-size`      | Size of orthographic projection                                              | `10`          | `10`          |
| `primary`         | Is this camera the primary?                                                  | `true`        | `false`       |
| `exposure`        | Controls the camara exposure                                                 | `1.0`         | `1.0`         |
| `bloom-intensity` | Controls the intensity of the Light Bloom                                    | `1.0`         | `1.0`         |
| `bloom-threshold` | Controls the threshold value that a pixel must have to be considerd to bloom | `0.5`         | `0.2`         |
| `hbao-radius`     | Controls the radius in which samples are taken (defined in eye space)        | `1.5`         | `1.0`         |
| `hbao-intensity`  | Controls the intensity of HBAO                                               | `2.0`         | `1.5`         |
| `hbao-bias`       | Controls the angle bias, to avoid occlusion near the tangent plane (0-1)     | `0.2`         | `0.35`        |
| `hbao-sharpness`  | Controls the sharpness of the HBAO Blurring                                  | `1.0`         | `1.0`         |

More info about HBAO: https://developer.download.nvidia.com/presentations/2008/SIGGRAPH/HBAO_SIG08b.pdf and https://github.com/nvpro-samples/gl_ssao

## AnimationComponent

| **Property**      | **Description**                                                                          | **Example**           | **Default** |
|-------------------|------------------------------------------------------------------------------------------|-----------------------|-------------|
| `asset-file`      | File to load Animation from (should match the asset-file used in the Renderer Component) | `model.fbx`           | -           |
| `animation`       | Animation to use. (Must be included in the Asset-File)                                   | `Armature\|Animation` | -           |
| `animation-speed` | Animation speed. (Can also be negative)                                                  | `1.0`                 | `1.0`       |
| `auto-play`       | Auto play the animation.                                                                 | `true`                | `true`      |
| `loop`            | Should the animation loop?                                                               | `true`                | `true`      |

## ScriptComponent

| **Property**  | **Description**                    | **Example**    | **Default** |
|---------------|------------------------------------|----------------|-------------|
| `script-name` | Name of the script to use          | `PlayerScript` | -           |

Any additional parameter set will be made accessible in the script via `NativeScript::getParameterMap()`.

Scripts must be registered in `Main.cpp` like this:
```c++
application->registerNativeScript<Game::CameraScript>("cameraScript");
```

## DirectionalLightComponent

| **Property**   | **Description**                                                             | **Example** | **Default** |
|----------------|-----------------------------------------------------------------------------|-------------|-------------|
| `color`        | Color of the light                                                          | `1 1 1`     | `1 1 1`     |
| `intensity`    | Intensity of the light                                                      | `1`         | `1`         |
| `cast-shadows` | Should the light cast shadows? (Only one shadow casting light is supported) | `true`      | `true`      |

## PointLightComponent

| **Property** | **Description**                                | **Example** | **Default** |
|--------------|------------------------------------------------|-------------|-------------|
| `color`      | Color of the light                             | `1 1 1`     | `1 1 1`     |
| `intensity`  | Intensity of the light                         | `1`         | `1`         |
| `radius`     | Radius of influence for the light              | `5`         | `5`         |
| `falloff`    | Falloff: 0 -> light falloff, 1 -> hard falloff | `1`         | `1`         |

## SpotLightComponent

| **Property**  | **Description**                                | **Example** | **Default** |
|---------------|------------------------------------------------|-------------|-------------|
| `color`       | Color of the light                             | `1 1 1`     | `1 1 1`     |
| `intensity`   | Intensity of the light                         | `1`         | `1`         |
| `radius`      | Radius of influence for the light              | `5`         | `5`         |
| `falloff`     | Falloff: 0 -> light falloff, 1 -> hard falloff | `1`         | `1`         |
| `inner-angle` | Inner cone angle (deg)                         | `50`        | `30`        |
| `outer-angle` | Outer cone angle (deg)                         | `60`        | `35`        |

## SkyboxComponent

| **Property**  | **Description**                                | **Example**  | **Default** |
|---------------|------------------------------------------------|--------------|-------------|
| `hdri-path`   | Path to the environment map                    | `skybox.hdr` | -           |
| `intensity`   | Intensity of the environment map               | `1`          | `1`         |
| `lod`         | Level of detail to display                     | `1`          | `1`         |

## RigidBodyComponent

| **Property**          | **Description**                                      | **Example** | **Default** |
|-----------------------|------------------------------------------------------|-------------|-------------|
| `dynamic`             | Is the RigidBody dynamic                             | `true`      | `false`     |
| `kinematic`           | Is the RigidBody kinematic                           | `false`     | `false`     |
| `disable-gravity`     | Disable gravity                                      | `false`     | `false`     |
| `mass`                | Mass of the RigidBody (kg)                           | `10`        | `1`         |
| `linear-drag`         | Linear drag                                          | `0`         | `0`         |
| `angular-drag`        | Angular drag                                         | `0`         | `0`         |
| `collision-detection` | Collision detection type: `discrete` or `continuous` | `discrete`  | `discrete`  |

[PhysX Docs](https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/docs/RigidBodyOverview.html)

## BoxColliderComponent

| **Property** | **Description**           | **Example**   | **Default**                |
|--------------|---------------------------|---------------|----------------------------|
| `half-size`  | Half-size of the collider | `0.5 0.5 0.5` | `0.5 0.5 0.5`              |
| `offset`     | Offset of the collider    | `0 0 0`       | `0 0 0`                    |
| `trigger`    | Is trigger                | `false`       | `false`                    |
| `material`   | Physics material to use   | `phyMaterial` | `default-physics-material` |

[PhysX Docs](https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/docs/Geometry.html#boxes)

## SphereColliderComponent

| **Property** | **Description**         | **Example**   | **Default**                |
|--------------|-------------------------|---------------|----------------------------|
| `radius`     | Radius of the collider  | `1`           | `1`                        |
| `offset`     | Offset of the collider  | `0 0 0`       | `0 0 0`                    |
| `trigger`    | Is trigger              | `false`       | `false`                    |
| `material`   | Physics material to use | `phyMaterial` | `default-physics-material` |

[PhysX Docs](https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/docs/Geometry.html#spheres)

## CapsuleColliderComponent

| **Property**  | **Description**             | **Example**   | **Default**                |
|---------------|-----------------------------|---------------|----------------------------|
| `radius`      | Radius of the collider      | `1`           | `1`                        |
| `half-heigth` | Half-Height of the collider | `0.5`         | `0.5`                      |
| `offset`      | Offset of the collider      | `0 0 0`       | `0 0 0`                    |
| `trigger`     | Is trigger                  | `false`       | `false`                    |
| `material`    | Physics material to use     | `phyMaterial` | `default-physics-material` |

[PhysX Docs](https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/docs/Geometry.html#capsules)

## TriangleColliderComponent

| **Property** | **Description**               | **Example**   | **Default**                |
|--------------|-------------------------------|---------------|----------------------------|
| `asset-file` | File to load 3D-Asset         | `model.fbx`   | -                          |
| `mesh-node`  | Node of the asset-file to use | `0.5`         | -                          |
| `trigger`    | Is trigger                    | `false`       | `false`                    |
| `material`   | Physics material to use       | `phyMaterial` | `default-physics-material` |

[PhysX Docs](https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/docs/Geometry.html#triangle-meshes)


## ConvexColliderComponent

| **Property** | **Description**               | **Example**   | **Default**                |
|--------------|-------------------------------|---------------|----------------------------|
| `asset-file` | File to load 3D-Asset         | `model.fbx`   | -                          |
| `mesh-node`  | Node of the asset-file to use | `0.5`         | -                          |
| `trigger`    | Is trigger                    | `false`       | `false`                    |
| `material`   | Physics material to use       | `phyMaterial` | `default-physics-material` |

[PhysX Docs](https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/docs/Geometry.html#convex-meshes)


## CharacterControllerComponent

| **Property**       | **Description**                     | **Example** | **Default** |
|--------------------|-------------------------------------|-------------|-------------|
| `has-gravity`      | Radius of the collider              | `true`      | `true`      |
| `step-offset`      | Step offset for the Controller      | `0.1`       | `0.0`       |
| `step-down-offset` | Step-down offset for the Controller | `0.1`       | `0.0`       |
| `slope-limit`      | Slope limit for the Controller      | `30`        | `0.0`       |

[PhysX CharacterController Docs](https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/docs/CharacterControllers.html)

## UiCanvasComponent2D

This component doesn't have direct properties. One can specify which UI Elements it should contain by adding XML child nodes.

All elements have the following properties:

| **Property** | **Description**                        | **Example**  | **Default** |
|--------------|----------------------------------------|--------------|-------------|
| `id`         | ID to find the Element. (required)     | `element-id` | -           |
| `top`        | Distance to top off the window         | `0`          | `0.0`       |
| `bottom`     | Distance bottom top off the window     | `0`          | -           |
| `left`       | Distance left top off the window       | `0`          | -           |
| `right`      | Distance right top off the window      | `0`          | -           |
| `x-align`    | Possible values: `left, center, right` | `left`       | `center`    |
| `y-align`    | Possible values: `top, center, bottom` | `top`        | `center`    |
| `z-index`    | Used for depth sorting                 | `1`          | `0`         |

For lengths and sizes following units can be used:

- Pixel: Just write the number - `10`
- ViewportWidthPercent: `10vw`
- ViewportHeightPercent: `10vh`

### UICircle

| **Property** | **Description**        | **Example** | **Default** |
|--------------|------------------------|-------------|-------------|
| `line-width` | Outline width          | `1`         | `0`         |
| `line-color` | Color of the outline   | `1 0 0 1`   | `0 0 0 1`   |
| `texture`    | Texture to use         | `image.png` | -           |
| `width`      | Diameter of the circle | `30`        | `0`         |

### UiRect

| **Property** | **Description**      | **Example** | **Default** |
|--------------|----------------------|-------------|-------------|
| `line-width` | Outline width        | `1`         | `0`         |
| `line-color` | Color of the outline | `1 0 0 1`   | `0 0 0 1`   |
| `texture`    | Texture to use       | `image.png` | -           |
| `width`      | Width of the rect    | `30`        | `0`         |
| `height`     | Height of the rect   | `30`        | `0`         |

### UIText

| **Property** | **Description** | **Example**  | **Default** |
|--------------|-----------------|--------------|-------------|
| `size`       | Font size       | `10`         | `0`         |
| `text`       | Text            | `Some Text`  | `""`        |
| `color`      | Text color      | `1 0 0 1`    | `0 0 0 1`   |
| `font`       | Font to use     | `roboto.ttf` | -           |

### AudioListenerComponent

| **Property** | **Description**                     | **Example** | **Default** |
|--------------|-------------------------------------|-------------|-------------|
| `volume`     | Gain of the listener                | `0.8`       | `1`         |
| `active`     | Indicates if the Listener is active | `true`      | `true`      |

### AudioComponent

| **Property**     | **Description**                                                            | **Example**    | **Default** |
|------------------|----------------------------------------------------------------------------|----------------|-------------|
| `asset-file`     | The sound that should be played                                            | `my-sound.wav` | -           |
| `volume`         | Volume to play the sound at                                                | `0.5`          | `1`         |
| `pitch`          | Pitch to use to play the sound                                             | `0.8`          | `1`         |
| `looping`        | Indicate if the sound should loop when finished playing                    | `true`         | `false`     |
| `play-on-attach` | If true, the sound will start to play as soon as the component is attached | `true`         | `false`     |
| `auto-destroy`   | If true, the component will be destroyed when the sound finishes playing   | `true`         | `false`     |

### LodDistanceComponent

| **Property**    | **Description**                                                                                                     | **Example** | **Default** |
|-----------------|---------------------------------------------------------------------------------------------------------------------|-------------|-------------|
| `lod-distances` | Defines the distances at which LODs (if available) are switches on renderer components attached to the same entity. | `5, 10, 50` | -           |
