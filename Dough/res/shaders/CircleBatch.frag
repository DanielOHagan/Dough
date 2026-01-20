#version 450
#extension GL_ARB_separate_shader_objects : enable

const int TEX_COUNT = 8;
layout (set = 1, binding = 0) uniform sampler2D samplers[TEX_COUNT];

layout (location = 0) in vec4 vFragColour;
layout (location = 1) in vec2 vTexCoord;
layout (location = 2) in vec2 vDecorations;
layout (location = 3) in vec2 vQuadBound;
layout (location = 4) in float vTexIndex;

layout (location = 0) out vec4 outColour;

void main() {
	//Fade MUST have a non-zero value
	const float fade = max(vDecorations.y, 0.0001);
	//Calculate the distance to vQuadBound and inverse it because vQuadBound is on the "outside" of the circle.
	float inverseDistance = 1.0 - length(vQuadBound);
	float decorationResult = smoothstep(0.0, fade, inverseDistance);
	decorationResult *= smoothstep(vDecorations.x + fade, vDecorations.x, inverseDistance);

	//smoothstep() clamps the inverseDistance to 0.0 if outside of the circle
	if (decorationResult == 0.0) {
		//Discard to help with transparency issues that can't be solved without sorting circles into other batches.
		discard;
	}

	outColour = texture(samplers[int(vTexIndex)], vTexCoord) * vFragColour;
	outColour.a *= decorationResult;
}
