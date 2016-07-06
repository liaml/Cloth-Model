//
//  LargeVM.hpp
//  Cloth M Alpha
//
//  Created by Liam Lees on 3/26/16.
//  Copyright © 2016 Liam Lees. All rights reserved.
//

#ifndef LargeVM_hpp
#define LargeVM_hpp

// OpenGL Extension Wrangler Library
#include <GL/glew.h>

// OpenGL Mathematics Library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

// C/C++ Standard Libraries
#include <vector>

template<class T>
class LargeVM {
    
private:
    
    std::vector<T> _vec;
    
public:
    
    LargeVM(void) {
        
    }
    
    LargeVM(const LargeVM& other) {
        
        _vec.resize(other._vec.size());
        memcpy(&_vec[0], &(other._vec[0]), sizeof(other._vec[0]) * other._vec.size());
    }
    
    GLvoid Resize(const GLsizei size) {
        
        _vec.resize(size);
    }
    
    GLvoid Clear(GLboolean isIdentity = GL_FALSE) {
        
        memset(&_vec[0], 0, sizeof(T) * _vec.size());
        
        if (isIdentity) {
            
            for (GLsizei i = 0; i < _vec.size(); i++) {
                
                _vec[1] = T(0);
            }
        }
    }
    
    T& operator[](GLuint index) {
        
        return _vec[index];
    }
    
    friend LargeVM<glm::vec3> operator*(const LargeVM<glm::mat3> other, const LargeVM<glm::vec3> v);
    
    friend LargeVM<glm::vec3> operator*(const GLfloat f, const LargeVM<glm::vec3> other);
    
    friend LargeVM<glm::vec3> operator-(const LargeVM<glm::vec3> vecA, const LargeVM<glm::vec3> vecB);
    
    friend LargeVM<glm::vec3> operator*(const LargeVM<glm::vec3> vecA, const LargeVM<glm::vec3> vecB);
    
    friend LargeVM<glm::vec3> operator+(const LargeVM<glm::vec3> vecA, const LargeVM<glm::vec3> vecB);
    
    friend LargeVM<glm::mat3> operator*(const GLfloat f, const LargeVM<glm::mat3> other);
    
    friend LargeVM<glm::mat3> operator-(const LargeVM<glm::mat3> vecA, const LargeVM<glm::mat3> vecB);
    
    friend LargeVM<glm::vec3> operator/(const GLfloat f, const LargeVM<glm::vec3> v);
    
    friend GLfloat dot(const LargeVM<glm::vec3> vecA, const LargeVM<glm::vec3> vecB);
};

#endif /* LargeVM_hpp */
