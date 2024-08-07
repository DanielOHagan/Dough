#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 1, binding = 0) uniform sampler2D samplers[8];

layout (location = 0) in vec4 vFragColour;
layout (location = 1) in vec2 vTexCoord;
layout (location = 2) in float vTexIndex;

layout (location = 0) out vec4 outColour;

void main() {
	outColour = texture(samplers[int(vTexIndex)], vTexCoord) * vFragColour;

	//Discard to help with transparency issues that can't be solved without sorting quads into other batches.
	if (outColour.a == 0.0) {
		discard;
	}
}
