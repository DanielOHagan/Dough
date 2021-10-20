#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 1) uniform sampler2D texSampler;
//layout (binding = 2) uniform sampler2D texSampler2;

layout (location = 0) in vec3 vFragColour;
layout (location = 1) in vec2 vTexCoord;
layout (location = 2) in float vTexIndex;

layout (location = 0) out vec4 outColour;


void main() {
	//if (vTexIndex > 0.0) {
		//outColour = texture(texSampler, vTexCoord);
	//} else {
		//outColour = texture(texSampler2, vTexCoord);
	//}

	outColour = texture(texSampler, vTexCoord);

	//outColour = vec4(vFragColour, 1.0);
}