#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform UniformBufferObject {
	mat4 proj;
} ubo;

layout (location = 0) in vec2 mVertPos;
layout (location = 1) in vec3 mColour;

layout (location = 0) out vec3 vFragColour;

void main() {
	gl_Position = ubo.proj * vec4(mVertPos, 1.0, 1.0);
	vFragColour = mColour;
}
