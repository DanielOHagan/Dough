#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 1, binding = 0) uniform sampler2D msdfSamplers[8];

layout (location = 0) in vec4 vFragColour;
layout (location = 1) in vec2 vTexCoords;
layout (location = 2) in float vTexIndex;

layout (location = 0) out vec4 outColour;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange() {
    //NOTE:: screenPxRange is 2.0 because 2.0 is used when creating the MSDF atlasses.
    const float screenPxRange = 2.0;
    vec2 unitRange = vec2(screenPxRange) / vec2(textureSize(msdfSamplers[int(vTexIndex)], 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(vTexCoords);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

void main() {
    vec3 msd = texture(msdfSamplers[int(vTexIndex)], vTexCoords).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange() * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    outColour = mix(vec4(0.0), vFragColour, opacity) * vFragColour;
}
