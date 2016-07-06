#version 330 core

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;

struct LightSource {
    
    vec3 ambientColour;
    vec3 diffuseColour;
    vec3 specularColour;
    
    vec3 position;
};

struct Material {
    
    vec3 ambientColour;
    vec3 diffuseColour;
    vec3 specularColour;
    float specularShininess;
};

uniform LightSource light;
uniform Material material;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec3 u_lightPos;

smooth out vec3 vertexNormal;
smooth out vec3 vertexPosition;

out vec3 lightIntensity;

void getEyeSpace(out vec3 norm, out vec4 position) {
    
    mat4 modelView = u_view * u_model;
    mat4 normalMatrix = transpose(inverse(modelView));
    
    norm = (normalize(normalMatrix * vec4(i_normal, 1.0))).xyz;
    position = modelView * vec4(i_position, 1.0);
}

vec3 phongModel(vec4 position, vec3 norm) {
    
    vec3 s = normalize(u_lightPos - position.xyz);
    vec3 v = normalize(-position.xyz);
    vec3 r = reflect(-s, norm);
    
    vec3 ambient = light.ambientColour * material.ambientColour;
    
    float SdotN = max(dot(s, norm), 0.0);
    
    vec3 diffuse = light.diffuseColour * material.diffuseColour * SdotN;
    
    vec3 specular = vec3(0.0);
    
    if (SdotN > 0.0)
        specular = light.specularColour * material.specularColour * pow(max(dot(r, v), 0.0), material.specularShininess);
    
    return(ambient + diffuse + specular);
}

void PVShader(void) {
    
    vec3 eyeNorm;
    vec4 eyePosition;
    
    // Get the position and normal in eye space
    getEyeSpace(eyeNorm, eyePosition);
    
    // Evaluate the position and normal in eye space
    lightIntensity = phongModel(eyePosition, eyeNorm);
    
    gl_Position = u_projection * u_view * u_model * vec4(i_position - vec3(0, 0.5,0), 1.0);
}

void main(void) {
    
    /// PER-VERTEX LIGHTING
    
    //PVShader();
    
    /// PER-PIXEL LIGHTING
    
    mat4 modelView = u_view * u_model;
    
    mat4 normalMatrix = transpose(inverse(modelView));
    vertexNormal = normalize(vec3(normalMatrix * vec4(i_normal, 1.0)));
    
    vertexPosition = vec3(modelView * vec4(i_position, 1.0));
    
    gl_Position = u_projection * modelView * vec4(i_position - vec3(0, 0.5,0), 1.0);
}