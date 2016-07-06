#version 330 core

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

in vec3 vertexNormal;
in vec3 vertexPosition;

in vec3 lightIntensity;

out vec4 FragColour;

vec3 ads(void) {
    
    vec3 n = normalize(vertexNormal);
    vec3 s = normalize(vec3(light.position) - vertexPosition);
    vec3 v = normalize(vec3(-vertexPosition));
    vec3 r = reflect(-s, n);
    
    vec3 ambient = vec3(0);
    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);
    
    ambient += light.ambientColour;
    diffuse += light.diffuseColour * max(dot(s, n), 0.0);
    specular += light.specularColour * pow(max(dot(r, v), 0.0), material.specularShininess);
    
    ambient *= material.ambientColour;
    diffuse *= material.diffuseColour;
    specular *= material.specularColour;
    
    
    return(ambient, diffuse, specular);
}

vec3 phongModel(vec3 position, vec3 norm) {
    
    vec3 s = normalize(light.position - position);
    vec3 v = normalize(-position.xyz);
    vec3 r = reflect(-s, norm);
    
    vec3 ambient = light.ambientColour * material.ambientColour;
    
    float SdotN = max(dot(s, norm), 0.0);
    
    vec3 diffuse = light.diffuseColour * material.diffuseColour * SdotN;
    
    vec3 specular = vec3(0.0);
    
    if (SdotN > 0.0)
        specular = light.specularColour * material.specularColour * pow(max(dot(r, v), 0.0), material.specularShininess);
    
    return(normalize(ambient + diffuse + specular));
}

void main(void) {

    // TODO: Incorrect normals
    
    /// PER-VERTEX LIGHTING
    
    //FragColour = vec4(lightIntensity, 1.0);
    
    /// PER-PIXEL LIGHTING
    
    FragColour = vec4(phongModel(vertexPosition, normalize(vertexNormal)), 1.0);
}