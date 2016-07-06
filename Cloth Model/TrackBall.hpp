#ifndef TRACKBALL_HPP
#define TRACKBALL_HPP

#pragma once

// OpenGL Extensions Wrangler Library
#include <GL/glew.h>

// OpenGL Window & Peripherals Handling Library
#include <GLFW/glfw3.h>

// OpenGL Mathematics Library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/gtc/matrix_inverse.hpp>
#include <GLM/gtx/rotate_vector.hpp>

class TrackBall {
    
public:
    
    TrackBall(GLuint winWidth, GLuint winHeight, GLfloat rollSpeed = 1.0f, GLboolean xAxis = true, GLboolean yAxis = true);
    glm::vec3 toScreenCoord(GLdouble x, GLdouble y);
    
    GLvoid mouseButtonCallback(GLFWwindow* window, GLint button, GLint action, GLint mods);
    GLvoid cursorCallback(GLFWwindow* window, GLdouble x, GLdouble y);
    
    inline glm::mat4 createViewRotationMatrix();
    glm::mat4 createModelRotationMatrix(glm::mat4& viewMatrix);
    
private:
    
    GLuint _wWidth;
    GLuint _wHeight;
    
    GLint _mEvent;
    
    GLfloat _rSpeed;
    GLfloat _angle;
    
    glm::vec3 _lastPos;
    glm::vec3 _position;
    glm::vec3 _camAxis;
    
    GLboolean _xAxis;
    GLboolean _yAxis;
};

#endif /* TrackBall_hpp */
