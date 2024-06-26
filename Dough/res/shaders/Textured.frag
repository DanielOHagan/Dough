#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec4 vFragColour;
layout (location = 1) in vec2 vTexCoord;

layout (location = 0) out vec4 outColour;

void main() {
	outColour = texture(texSampler, vTexCoord);
}
