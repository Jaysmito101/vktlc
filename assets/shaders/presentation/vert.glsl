#version 450

#pragma shader_stage(vertex)

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform constants {
    float viewportWidth;
    float viewportHeight;
    float renderFrameWidth;
    float renderFrameHeight;
} PresentationPipelineConfig;

const vec2 positions[6] = vec2[6](
    vec2(-1.0, -1.0), // Bottom left
    vec2(1.0, -1.0),  // Bottom right
    vec2(-1.0, 1.0),  // Top left
    vec2(1.0, 1.0),   // Top right
    vec2(-1.0, 1.0),  // Top left (duplicate for triangle strip)
    vec2(1.0, -1.0)   // Bottom right (duplicate for triangle strip)
);

void main() {
    // Generate positions for a quad with 4 vertices
    float aspectRatio = PresentationPipelineConfig.viewportWidth / PresentationPipelineConfig.viewportHeight;
    float renderFrameAspectRatio = PresentationPipelineConfig.renderFrameWidth / PresentationPipelineConfig.renderFrameHeight;
  
    // Use the vertex index to determine the position and color
    float x = positions[gl_VertexIndex].x;
    float y = positions[gl_VertexIndex].y;

    float xScale = 1.0;
    float yScale = 1.0;

    if (renderFrameAspectRatio > aspectRatio) {
        yScale = aspectRatio / renderFrameAspectRatio;
    } else {
        xScale = renderFrameAspectRatio / aspectRatio;
    }

    gl_Position = vec4(x * xScale, y * yScale, 0.0, 1.0);
    fragColor = vec3(x * 0.5 + 0.5, y * 0.5 + 0.5, 1.0); // Color based on position
}