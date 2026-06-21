#ifndef MACROS_GLSL
#define MACROS_GLSL

#ifdef VULKAN
    #define PUSH_CONSTANT(STRUCT_NAME) layout(push_constant, std430) uniform STRUCT_NAME
    #define GET_INSTANCE_INDEX() gl_InstanceIndex
#else
    #define PUSH_CONSTANT(STRUCT_NAME) layout(binding = 16, std430) readonly buffer STRUCT_NAME
    #define GET_INSTANCE_INDEX() gl_InstanceID
#endif

#endif // MACROS_GLSL
