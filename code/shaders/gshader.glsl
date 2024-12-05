#version 330 core

layout(points) in; // Expecting points as input
layout(line_strip, max_vertices = 2) out; // Line strip with 2 vertices per normal

in vec3 worldPosition[];
in vec3 worldNormal[];

uniform mat4 viewProjMatrix;

void main() {
    // Starting point of the line (vertex position)
    vec4 start = vec4(worldPosition[0], 1.0);
    gl_Position = viewProjMatrix * start;
    EmitVertex();

    // End point of the line (vertex position + normal direction)
    vec4 end = vec4(worldPosition[0] + worldNormal[0] * 0.2, 1.0); // Scale normal for visibility
    gl_Position = viewProjMatrix * end;
    EmitVertex();

    EndPrimitive();
}
