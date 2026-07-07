#ifndef MACROS_GLSL
#define MACROS_GLSL

#ifdef VULKAN
    #define PUSH_CONSTANT(STRUCT_NAME) layout(push_constant, std430) uniform STRUCT_NAME
    #define GET_INSTANCE_INDEX() gl_InstanceIndex
    #define UNIFORM_LAYOUT(BINDING, SET) layout(binding = BINDING, set = SET)
    #define UNIFORM_LAYOUT_STD140(BINDING, SET) layout(binding = BINDING, set = SET, std140)
    #define UNIFORM_LAYOUT_STD430(BINDING, SET) layout(binding = BINDING, set = SET, std430)
#else
    #define PUSH_CONSTANT(STRUCT_NAME) layout(binding = 16, std430) readonly buffer STRUCT_NAME
    #define GET_INSTANCE_INDEX() gl_InstanceID
    #define UNIFORM_LAYOUT(BINDING, SET) layout(binding = BINDING)
    #define UNIFORM_LAYOUT_STD140(BINDING, SET) layout(binding = BINDING, std140)
    #define UNIFORM_LAYOUT_STD430(BINDING, SET) layout(binding = BINDING, std430)
#endif

#endif // MACROS_GLSL
