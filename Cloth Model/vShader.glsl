#version 330 core

// Attributes
layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;

// Light & Camera Position
uniform vec3 u_lightPosition;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 vertexNormal;

void main(void) {

    gl_Position = (u_projection * u_view * u_model) * vec4(i_position, 1) - vec4(vec3(0, 1.5, 0), 0);

    mat3 normalMatrix = mat3(transpose(inverse(u_view * u_model)));
    vertexNormal = normalize(vec3(u_projection * vec4(normalMatrix * i_normal, 1.0)));
}