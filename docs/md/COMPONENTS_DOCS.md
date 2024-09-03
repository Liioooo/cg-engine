# Available Components

## TransformComponent

| **Property** | **Description**                    | **Example** | **Default** |
|--------------|------------------------------------|-------------|-------------|
| `position`   | Translation of the entity          | `0 0 0`     | `0 0 0`     |
| `rotation`   | Local rotation of the entity (deg) | `0 0 0`     | `0 0 0`     |
| `scale`      | Local scale of the entity          | `0 0 0`     | `1 1 1`     |

## MeshRendererComponent

| **Property**   | **Description**                                                 | **Example**           | **Default** |
|----------------|-----------------------------------------------------------------|-----------------------|-------------|
| `asset-file`   | File to load 3D-Asset                                           | `model.fbx`           | -           |
| `mesh`         | Built in mesh                                                   | `CG_SphereMesh_16_16` | -           |
| `material`     | Material for this mesh. Overrides materials from asset files.   | `MyMaterial`          | -           |
| `cast-shadows` | Defines if this object casts shadows.                           | `true`                | `true`      |
| `mesh-nodes`   | List of Nodes to render. Nothing means: everything is rendered. | `RootNode, Node1`     | -           |

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

| **Property**      | **Description**                                                 | **Example**           | **Default** |
|-------------------|-----------------------------------------------------------------|-----------------------|-------------|
| `asset-file`      | File to load 3D-Asset                                           | `model.fbx`           | -           |
| `material`        | Material for this mesh. Overrides materials from asset files.   | `MyMaterial`          | -           |
| `cast-shadows`    | Defines if this object casts shadows.                           | `true`                | `true`      |
| `mesh-nodes`      | List of Nodes to render. Nothing means: everything is rendered. | `RootNode, Node1`     | -           |
| `animation`       | Animation to use. (Must be included in the Asset-File)          | `Armature\|Animation` | -           |
| `animation-speed` | Animation speed. (Can also be negative)                         | `1.0`                 | `1.0`       |
| `auto-play`       | Auto play the animation.                                        | `true`                | `true`      |
| `loop`            | Should the animation loop?                                      | `true`                | `true`      |

## CameraComponent

| **Property** | **Description**                           | **Example**   | **Default**   |
|--------------|-------------------------------------------|---------------|---------------|
| `projection` | Projection: `perspective` or `orthogonal` | `perspective` | `perspective` |
| `near`       | Near plane                                | `0.1`         | `0.1`         |
| `far`        | Far plane                                 | `100`         | `100`         |
| `fov`        | Field of View (deg)                       | `60`          | `60`          |
| `ortho-size` | Size of orthographic projection           | `10`          | `10`          |
| `primary`    | Is this camera the primary?               | `true`        | `false`       |

## ScriptComponent

| **Property**  | **Description**                    | **Example**    | **Default** |
|---------------|------------------------------------|----------------|-------------|
| `script-name` | Name of the script to use          | `PlayerScript` | -           |

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

| **Property**  | **Description**                | **Example** | **Default** |
|---------------|--------------------------------|-------------|-------------|
| `has-gravity` | Radius of the collider         | `true`      | `true`      |
| `step-offset` | Step offset for the Controller | `0.1`       | `0.0`       |
| `slope-limit` | Slope limit for the Controller | `30`        | `0.0`       |

[PhysX CharacterController Docs](https://nvidia-omniverse.github.io/PhysX/physx/5.1.3/docs/CharacterControllers.html)

## UiCanvasComponent

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
