#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices = 2) out;

uniform float length = 0.150;
uniform vec3 color = vec3(1.0, 1.0, 1.0);

in vec3 vertexNormal[];

out vec3 vertexColour;

void main(void) {
    
    vec3 normal = vertexNormal[0];
    
    vertexColour = abs(normal * 0.95);
    
    vec4 v0 = gl_in[0].gl_Position;
    gl_Position = v0;
    EmitVertex();
    
    vec4 v1 = v0 + vec4(normal * length, 0.0);
    gl_Position =  v1;
    EmitVertex();
    
    EndPrimitive();
}