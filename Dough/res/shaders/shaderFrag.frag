#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 1) uniform sampler2D texSampler;

layout (location = 0) in vec3 vFragColour;
layout (location = 1) in vec2 vTexCoord;
layout (location = 2) in float vTexIndex;

layout (location = 0) out vec4 outColour;

void main() {
	//outColour = texture(texSampler, vTexCoord);

	//DEBUG:: Vertex colour for testing coords
	if (vTexIndex == 1.0) {
		outColour = texture(texSampler, vTexCoord);
	} else {
		outColour = vec4(vFragColour, 1.0);
	}
}
