#version 460

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} gubo;

layout(push_constant) uniform Constants {
    mat4 model;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNorm;

void main() {
    gl_Position = gubo.proj * gubo.view * pc.model * vec4(inPosition, 1.0);
}