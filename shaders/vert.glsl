#version 450

#pragma shader_stage(vertex)

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inNormal;
layout (location = 2) in vec4 inTexCoord;
layout (location = 3) in vec4 inColor;
layout (location = 4) in vec4 inBoneWeights;
layout (location = 5) in ivec4 inBoneIDs;

layout(location = 0) out vec3 fragColor;

void main() {
    // gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    // fragColor = colors[gl_VertexIndex];

    gl_Position = inPosition;
    fragColor = inColor.rgb;
}