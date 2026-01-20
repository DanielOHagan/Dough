#version 450
#extension GL_ARB_separate_shader_objects : enable

const int TEX_COUNT = 8;
layout (set = 1, binding = 0) uniform sampler2D samplers[TEX_COUNT];

layout (location = 0) in vec4 vFragColour;
layout (location = 1) in vec2 vTexCoord;
layout (location = 2) in float vTexIndex;

layout (location = 0) out vec4 outColour;

void main() {
	outColour = texture(samplers[int(vTexIndex)], vTexCoord) * vFragColour;

	//Discard fragment if too far from the glyph, reduces space taken up by rendered glyhp (no outlining quad) but can cause glyph to look a little "bumpy".
	// To completely remove the "bumpiness" then remove this discard statement.
	if (outColour.a < 0.1) discard;
}
