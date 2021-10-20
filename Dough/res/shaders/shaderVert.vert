#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout (location = 0) in vec2 mVertPos;
layout (location = 1) in vec3 mColour;
layout (location = 2) in vec2 mTexCoord;
layout (location = 3) in float mTexIndex;

layout (location = 0) out vec3 vFragColour;
layout (location = 1) out vec2 vTexCoord;
layout (location = 2) out float vTexIndex;

void main() {
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(mVertPos, 0.0, 1.0);
	vFragColour = mColour;
	vTexCoord = mTexCoord;
	vTexIndex = mTexIndex;
}
