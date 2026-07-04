#ifndef CUSTOM_PIPELINE_DATA_PC_GLSL
#define CUSTOM_PIPELINE_DATA_PC_GLSL

#include "Macros.glsl"

PUSH_CONSTANT(CustomPipelineData) {
    mat4 transform;
} pc_customPipelineData;

#endif // CUSTOM_PIPELINE_DATA_PC_GLSL
