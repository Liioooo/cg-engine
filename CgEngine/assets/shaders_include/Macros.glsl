#ifndef MACROS_GLSL
#define MACROS_GLSL

#ifdef VULKAN
    #define MATERIAL_PC layout(push_constant) uniform MaterialPushConstants
#else
    #define MATERIAL_PC uniform struct MaterialPushConstants
#endif

#ifdef VULKAN
    #define PUSH_CONSTANT(STRUCT_NAME, LOCATION) layout(push_constant) uniform STRUCT_NAME
#else
    #define PUSH_CONSTANT(STRUCT_NAME, LOCATION) layout(location = LOCATION) uniform struct STRUCT_NAME
#endif

#endif // MACROS_GLSL
