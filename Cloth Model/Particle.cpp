#include "Particle.hpp"

using namespace glm;


Particle::Particle(void)
{
}

Particle::Particle(vec3 pos) : _position(pos), _lastPos(pos) {
    
    _mass = 5.0f;
    
    _velocity = vec3(NULL);
    _lastVel = _velocity;
    
    _acceleration =  vec3(NULL);
    _force = vec3(NULL);
    
    _accumulatedNormal = vec3(NULL);
    
    _movable = GL_TRUE;
    _fixedFlag = GL_FALSE;
    
    _collision = GL_FALSE;
    
    _particleIndex = NULL;
}

Particle::~Particle(void)
{
}

GLvoid Particle::setPosition(vec3 pos) {
    if (_movable) {
        _position = pos;
    }
}

GLvoid Particle::setVelocity(vec3 vel) {
    _velocity = vel;
}

GLvoid Particle::setNewPosition(vec3 pos) {
    if (_movable) {
        vec3 buffer = _position;
        _position = pos;
        _lastPos = buffer;
    }
}

GLvoid Particle::offsetPosition(vec3 pos) {
    _position = _position + pos;
}

GLvoid Particle::setNewVelocity(vec3 vel) {
    
    vec3 buffer = _velocity;
    _velocity = vel;
    _lastVel = buffer;
}

GLvoid Particle::offsetVelocity(vec3 vel) {
    _velocity = _velocity + vel;
}

vec3& Particle::getPosition(void) {
    return _position;
}

vec3& Particle::getLPosition(void) {
    return _lastPos;
}

GLvoid Particle::setFlag(GLboolean f) {
    _movable = !(f);
}

GLboolean& Particle::getFlag(void) {
    return _movable;
}

GLfloat& Particle::getMass(void) {
    return _mass;
}

GLvoid Particle::setMass(GLfloat m) {
    _mass = m;
}

vec3& Particle::getVelocity(void) {
    return _velocity;
}

vec3& Particle::getLVelocity(void) {
    return _lastVel;
}

inline GLvoid Particle::calcAcceleration(void) {
    _acceleration = _force / _mass;
}

GLvoid Particle::resetForce(void) {
    _force = vec3(0);
    _acceleration = vec3(0);
}

GLvoid Particle::setForce(vec3 f) {
    _force = f;
    _acceleration = f / _mass;
}

GLvoid Particle::addForce(vec3 f) {
    _force += f;
    _acceleration += f / _mass;
}

vec3& Particle::getForce(void) {
    return _force;
}

vec3& Particle::getAcceleration(void) {
    return _acceleration;
}

GLvoid Particle::resetNormal(void) {
    _accumulatedNormal = vec3(0);
}

GLvoid Particle::addToNormal(vec3 n) {
    _accumulatedNormal += normalize(n);
}

vec3& Particle::getNormal(void) {
    return _accumulatedNormal;
}

GLvoid Particle::setIndex(GLuint in) {
    _particleIndex = in;
}

GLuint& Particle::getIndex(void) {
    return _particleIndex;
}

GLvoid Particle::setCol(GLboolean flag) {
    _collision = flag;
}

GLboolean& Particle::getCollFlag(void) {
    return _collision;
}


