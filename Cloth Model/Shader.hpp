// Header Guards
#ifndef SHADER_HPP
#define SHADER_HPP

#pragma once

// GLEW (OpenGL Extension Wrangler Library)
#include <GL/glew.h>

// OpenGL Mathematics Library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

// Standard C/C++ Libraries
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>


class Shader
{
    
public:
    
    Shader(void);
    ~Shader(void);
    
    GLuint ProgramID;
    
    GLuint GetProgramID(void);
    GLuint LoadShaders(const char* vFPath, const char* gFPath, const char * fFPath);
    GLuint LoadShaders(const char* vFPath, const char * fFPath);

    GLvoid UseProgram(GLuint ProgramID);
    
    GLvoid initShaders(void);

private:
    
    glm::vec4 lightPos0;
    glm::vec4 lightPos1;    
  
};

#endif