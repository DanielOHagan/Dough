#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform UniformBufferObject {
	mat4 projView;
} ubo;

layout (location = 0) in vec3 mVertPos;
layout (location = 1) in vec4 mColour;

layout (location = 0) out vec4 vFragColour;

void main() {
	gl_Position = ubo.projView * vec4(mVertPos, 1.0);
	vFragColour = mColour;
}
