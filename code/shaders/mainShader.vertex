#version 330 core

in vec3 vertexPosition;

uniform mat4 model;
uniform mat4 viewProjection;

void main()
{
    gl_Position = viewProjection * model * vec4(vertexPosition, 1.0);
}
