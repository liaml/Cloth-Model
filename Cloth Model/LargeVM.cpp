#include "LargeVM.hpp"

using namespace std;
using namespace glm;

LargeVM<vec3> operator*(const LargeVM<mat3> other, const LargeVM<vec3> v) {
    
    LargeVM<vec3> tmp(v);
    
    for (GLsizei i = 0; i < v._vec.size(); i++) {
        
        tmp._vec[i] = other._vec[i] * v._vec[i];
    }
    
    return tmp;
}

LargeVM<vec3> operator*(const LargeVM<vec3> other, const LargeVM<vec3> v) {
    
    LargeVM<vec3> tmp(v);
    
    for (GLsizei i = 0; i < v._vec.size(); i++) {
        
        tmp._vec[i] = other._vec[i] * v._vec[i];
    }
    
    return tmp;
}

LargeVM<vec3> operator*(const GLfloat f, const LargeVM<vec3> other) {
    
    LargeVM<vec3> tmp(other);
    
    for (GLsizei i = 0; i < other._vec.size(); i++) {
        
        tmp._vec[i] = other._vec[i] * f;
    }
    
    return tmp;
}

LargeVM<mat3> operator*(const GLfloat f, const LargeVM<mat3> other) {
    
    LargeVM<mat3> tmp(other);
    
    for(GLsizei i = 0; i < other._vec.size(); i++) {
        
        tmp._vec[i] = other._vec[i] * f;
    }
    
    return tmp;
}

LargeVM<vec3> operator-(const LargeVM<vec3> Va, const LargeVM<vec3> Vb) {
    
    LargeVM<vec3> tmp(Va);
    
    for (GLsizei i = 0; i < Va._vec.size(); i++) {
        
        tmp._vec[i] = Va._vec[i] - Vb._vec[i];
    }
    
    return tmp;
}

LargeVM<mat3> operator-(const LargeVM<mat3> Va, const LargeVM<mat3> Vb) {
    
    LargeVM<mat3> tmp(Va);
    
    for(GLsizei i = 0; i < Va._vec.size(); i++) {
        
        tmp._vec[i] = Va._vec[i] - Vb._vec[i];
    }
    
    return tmp;
}


LargeVM<vec3> operator+(const LargeVM<vec3> Va, const LargeVM<vec3> Vb) {
    
    LargeVM<vec3> tmp(Va);
    
    for (GLsizei i = 0; i < Va._vec.size(); i++) {
        
        tmp._vec[i] = Va._vec[i] + Vb._vec[i];
    }
    
    return tmp;
}

LargeVM<vec3> operator/(const GLfloat f, const LargeVM<vec3> v) {
    
    LargeVM<vec3> tmp(v);
    
    for (GLsizei i = 0; i < v._vec.size(); i++) {
        
        tmp._vec[i] = v._vec[i] / f;
    }
    
    return tmp;
}

GLfloat dot(const LargeVM<vec3> Va, const LargeVM<vec3> Vb) {
    
    GLfloat sum = 0;
    
    for (GLsizei i = 0; i < Va._vec.size(); i++) {
        
        sum += dot(Va._vec[i], Vb._vec[i]); // GLM Quaternion Dot Product
    }
    
    
    return sum;
}
