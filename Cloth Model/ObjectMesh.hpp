#ifndef OBJECTMESH_HPP
#define OBJECTMESH_HPP

#pragma once

// OpenGL Extension Wrangler Library
#include <GL/glew.h>

// OpenGL Window & Peripherals Handling Library
#include <GLFW/glfw3.h>

// OpenGL Mathematics Library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

// C/C++ Standard Libraries
#include <vector>

// Dependancies
#include "Shader.hpp"

using namespace std;
using namespace glm;

class ObjectMesh {
    
public:
    
    ObjectMesh(void);
    ~ObjectMesh(void);
    
    GLvoid initPlane(GLint gSize2, GLfloat sc);
    GLvoid renderPlane(Shader* shader);
    
    GLvoid initSphere(GLuint slices, GLuint stacks, GLfloat radius, vec3 position);
    GLvoid renderSphere(Shader* shader);
    
    GLvoid spherePositionUpdate(vec3 pos);
    vec3& getSpherePosition(void);
    
    GLvoid setSphereRadius(GLfloat rad);
    GLfloat& getSphereRadius(void);
    
private:
    
    struct Vertex {
        
        vec3 position;
        vec3 normal;
        vec2 uv;
    };
    
    vector<vec3> planeData;
    
    vector<Vertex> sphereData;
    vector<GLuint> sphereIndices;
    
    vec3 spherePosition;
    GLfloat sphereRadius;
    
    GLuint PlaneVAO;
    GLuint SphereVAO;
    
    GLuint VBO;
    GLuint EBO;
    
    GLboolean drawWire;
};

#endif /* ObjectMesh_hpp */
