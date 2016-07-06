#include "ObjectMesh.hpp"

#define offset 0

ObjectMesh::ObjectMesh(void)
{
    
    PlaneVAO = NULL;
    SphereVAO = NULL;
    
    VBO = NULL;
    EBO = NULL;
    
    sphereRadius = NULL;
    spherePosition = vec3(NULL);
    
    drawWire = false;
}

ObjectMesh::~ObjectMesh(void)
{
}

GLvoid ObjectMesh::spherePositionUpdate(vec3 pos) {
    
    spherePosition = pos;
    
    for (auto s = sphereData.begin(); s != sphereData.end(); s++) {
        s->position += spherePosition;
    }
}

vec3& ObjectMesh::getSpherePosition(void) {
    return spherePosition;
}

GLvoid ObjectMesh::setSphereRadius(GLfloat r) {
    sphereRadius = r;
}

GLfloat& ObjectMesh::getSphereRadius(void) {
    return sphereRadius;
}

GLvoid ObjectMesh::initPlane(GLint gSize2, GLfloat sc) {
    
    vec3 normal = vec3(0, 1, 0);
    
    for (GLint i = -gSize2; i <= gSize2; i++) {
        
        planeData.push_back(vec3((GLfloat)i, offset, (GLfloat)-gSize2) * sc);
        planeData.push_back(vec3((GLfloat)i, offset, (GLfloat)gSize2) * sc);
        
        planeData.push_back(vec3((GLfloat)-gSize2, offset, (GLfloat)i) * sc);
        planeData.push_back(vec3((GLfloat)gSize2, offset, (GLfloat)i) * sc);
    }
}

GLvoid ObjectMesh::renderPlane(Shader* shader) {
    
    glPolygonMode(GL_FRONT_AND_BACK, (drawWire ? GL_LINE : GL_FILL));
    
    glGenVertexArrays(1, &PlaneVAO);
    glBindVertexArray(PlaneVAO);
    
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    GLuint positionAttributeLocation = glGetAttribLocation(shader->ProgramID, "i_position");
    glEnableVertexAttribArray(positionAttributeLocation);
    glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (const GLvoid *)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, planeData.size() * sizeof(vec3), value_ptr(planeData[0]), GL_STATIC_DRAW);
    
    glDrawArrays(GL_LINES, 0, (GLsizei)planeData.size());
    
    // Deallocate Memory
    glDisableVertexAttribArray(positionAttributeLocation);
    
    glDeleteVertexArrays(1, &PlaneVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

GLvoid ObjectMesh::initSphere(GLuint slices, GLuint stacks, GLfloat rad, vec3 pos) {
    
    sphereRadius = rad;
    spherePosition = pos;
    
    for (GLsizei j = 0; j <= stacks; j++) {
        
        GLfloat latitude = ((pi<GLfloat>() / stacks) * j) - half_pi<GLfloat>();
        GLfloat sinLat = sin(latitude);
        GLfloat cosLat = cos(latitude);
        
        for (GLsizei i = 0; i <= slices; i++) {
            
            GLfloat longitude = ((2 * pi<GLfloat>()) / slices) * i;
            GLfloat sinLong = sin(longitude);
            GLfloat cosLong = cos(longitude);
            
            vec3 normal = vec3(cosLong * cosLat, sinLat, sinLong * cosLat);
            vec3 position = (normal * sphereRadius) + pos;
            vec2 uv = vec2((j / (GLfloat)stacks), (i / (GLfloat)slices));
            
            Vertex v = {
                
                position,
                normal,
                uv
            };
            
            sphereData.push_back(v);
        }
    }
    
    for (GLsizei j = 0; j < stacks; j++) {
        
        GLuint index = 0;
        
        if (j > 0)
            sphereIndices.push_back(j * (slices + 1));
        
        for (GLsizei i = 0; i <= slices; i++) {
            
            index = i + (j * (slices + 1));
            
            sphereIndices.push_back(index);
            sphereIndices.push_back(index + (slices + 1));
        }
        
        if ((j + 1) < stacks)
            sphereIndices.push_back(index + (slices + 1));
    }
}

GLvoid ObjectMesh::renderSphere(Shader* shader) {
    
    glPolygonMode(GL_FRONT_AND_BACK, (drawWire ? GL_LINE : GL_FILL));
    
    glGenVertexArrays(1, &SphereVAO);
    glBindVertexArray(SphereVAO);
    
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    GLuint positionAttributeLocation = glGetAttribLocation(shader->ProgramID, "i_position");
    glEnableVertexAttribArray(positionAttributeLocation);
    glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0);
    
    GLuint normalAttributeLocation = glGetAttribLocation(shader->ProgramID, "i_normal");
    glEnableVertexAttribArray(normalAttributeLocation);
    glVertexAttribPointer(normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)sizeof(vec3));
    
    GLuint uvAttributeLocation = glGetAttribLocation(shader->ProgramID, "i_uv");
    glEnableVertexAttribArray(uvAttributeLocation);
    glVertexAttribPointer(uvAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)(sizeof(vec3) + sizeof(vec3)));
    
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(GLuint), &sphereIndices[0], GL_STATIC_DRAW);
    
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereData.size() * sizeof(Vertex), value_ptr(sphereData[0].position), GL_STATIC_DRAW);
    
    glBindVertexArray(SphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)sphereIndices.size(), GL_UNSIGNED_INT, 0);
    
    // Deallocate Memory
    glDisableVertexAttribArray(positionAttributeLocation);
    glDisableVertexAttribArray(normalAttributeLocation);
    glDisableVertexAttribArray(uvAttributeLocation);
    
    glDeleteVertexArrays(1, &SphereVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
