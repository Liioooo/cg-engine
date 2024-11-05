# Documentation

## Libraries

| Name      | Link                                      | Usage                                      |
|-----------|-------------------------------------------|--------------------------------------------|
| spdlog    | https://github.com/gabime/spdlog          | Logging in the Game (useful for debugging) |
| inih      | https://github.com/benhoyt/inih           | Reading of .ini files                      |
| glfw      | https://github.com/glfw/glfw              | Window management abstraction              |
| glm       | https://github.com/g-truc/glm             | OpenGL Mathematics (GLM)                   |
| glad      | https://glad.dav1d.de/                    | OpenGL extension loading                   |
| pugixml   | https://pugixml.org/                      | Loading and parsing of XML files           |
| stb_image | https://github.com/nothings/stb           | Loading textures from files                |
| assimp    | https://github.com/assimp/assimp          | 3D Model loading                           |
| PhysX     | https://github.com/NVIDIA-Omniverse/PhysX | 3D Physics                                 |
| FreeType  | https://freetype.org/index.html           | Font loading                               |

## Available Components

[List of Components](COMPONENTS_DOCS.md)

## Configuration of Resources

### `assets/settings.ini`
Could for example look like this:
```ini
[window]
width = 1280
height = 720
refresh_rate = 60
fullscreen = false
title = RealTimeRendering
icon = path/to/some/icon.png
v_sync = false

[application]
debug_show_physics_colliders = false
debug_show_bounding_boxes = false
debug_show_normals = false
debug_render_lines = true
anisotropic_filtering = 16.0
shadow_map_resolution = 4096
enable_bloom = true
enable_hbao = true

[game]
startScene = scenes/start_scene.xml
```

Some properties can be changed at runtime (But they only work in Debug and Release, not Dist):
- `debug_show_physics_colliders`: F1
- `debug_show_normals`: F2
- `debug_show_bounding_boxes`: F3

### `assets/materials.xml`
Here you can define material to render.

```xml
<Materials>
    <Material name="MyMaterial">
        <Albedo>#ffffff</Albedo>
        <Roughness>1.0</Roughness>
        <Metalness>0</Metalness>
        <Emission>1.0</Emission>
        <EmissionColor>#ff0000</EmissionColor>
    </Material>
</Materials>
```

### `assets/physics-materials.xml`
Here you can define physics materials used by PhysX.

```xml
<Materials>
    <Material name="MyPhysicsMaterial">
        <StaticFriction>0.5</StaticFriction>
        <DynamicFricion>0.5</DynamicFricion>
        <Restitution>0.85</Restitution>
    </Material>
</Materials>
```

### `assets/prefabs.xml`
Here you can define Prefabs that can be used in Scene files, other Prefabs or instantiated from scripts.

```xml
<Prefabs>
    <Prefab name="myPrefab">
        <Components>
            <MeshRendererComponent mesh="CG_SphereMesh" />
        </Components>
        <Entity>
            <Components>
                <TransformComponent position="0 10 0" rotation="0 0 0" scale="0.25 0.25 0.25" />
                <MeshRendererComponent mesh="CG_SphereMesh" />
            </Components>
        </Entity>
    </Prefab>
</Prefabs>
```
The root of a Prefab cannot have a transform, because this will be set when instantiating it.

To use a Prefab in another Prefab or a Scene file you can do this:
```xml
<Scene>
    <Entity>
        <Components>
            <TransformComponent position="0 0 0" rotation="0 0 0" scale="1 1 1" />
            <CameraComponent primary="true" projection="perspective" near="0.1" far="500" fov="80" />
        </Components>
    </Entity>
    <Prefab name="myPrefab" position="0 3 0" rotation="0 0 0" scale="0.5 0.5 0.5" />
</Scene>
```

It can also be instantiated using scripts: `instantiatePrefab("coin", CgEngine::NoEntity);`

### `assets/shaders.xml`
Here you can define custom shaders that can be used in `CustomShaderRendererComponent` or in scripts.

```xml
<Shaders>
    <Shader name="my-shader" type="render">
        <Vertex>shaders/my-shader_vertex.glsl</Vertex>
        <Fragment>shaders/my-shader_fragment.glsl</Fragment>
        <Tcs>shaders/my-shader_tcs.glsl</Tcs>
        <Tes>shaders/my-shader_tes.glsl</Tes>
    </Shader>
    <Shader name="my-compute-shader" type="compute">
        <Path>shaders/my-compute-shader.glsl</Path>
    </Shader>
</Shaders>
```
