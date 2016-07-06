// Header Guards
#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#pragma once

// OpenGL Extensions Wrangler Library
#include <GL/glew.h>

// OpenGL Mathematics Library
#include <GLM/glm.hpp>

class Particle {
    
public:
    
    Particle(void);
    Particle(glm::vec3 pos);
    ~Particle(void);
    
    GLvoid setNewPosition(glm::vec3 pos);
    GLvoid setPosition(glm::vec3 p);
    GLvoid offsetPosition(glm::vec3 p);
    glm::vec3& getPosition(void);
    glm::vec3& getLPosition(void);
    
    GLvoid setNewVelocity(glm::vec3 vel);
    GLvoid setVelocity(glm::vec3 v);
    GLvoid offsetVelocity(glm::vec3 v);
    glm::vec3& getVelocity(void);
    glm::vec3& getLVelocity(void);
    
    
    GLvoid setForce(glm::vec3 f);
    GLvoid addForce(glm::vec3 f);
    GLvoid resetForce(void);
    glm::vec3& getForce(void);
    glm::vec3& getAcceleration(void);
    
    GLvoid setMass(GLfloat m);
    GLfloat& getMass(void);
    
    GLvoid addToNormal(glm::vec3 n);
    GLvoid resetNormal(void);
    glm::vec3& getNormal(void);
    
    GLvoid setFlag(GLboolean flag);
    GLboolean& getFlag(void);
    
    GLvoid setCol(GLboolean flag);
    GLboolean& getCollFlag(void);
    
    inline GLvoid calcAcceleration(void);
    
    GLvoid setIndex(GLuint in);
    GLuint& getIndex(void);
    
private:
    
    glm::vec3 _position;
    glm::vec3 _lastPos;
    glm::vec3 _velocity;
    glm::vec3 _lastVel;
    
    GLfloat _mass;
    
    glm::vec3 _acceleration;
    glm::vec3 _force;
    
    glm::vec3 _accumulatedNormal;
    
    GLboolean _movable;
    GLboolean _fixedFlag;
    
    GLboolean _collision;
    
    GLuint _particleIndex;
};

#endif /* Particles_hpp */