// Header Guards
#ifndef CLOTH_HPP
#define CLOTH_HPP

#pragma once

// OpenGL Extensions Wrangler Library
#include <GL/glew.h>

// OpenGL Mathematics Library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

// Simple OpenGL Image Loading Library
//#include <SOIL.h>

// OpenMP Multi-Processing Library
#include <omp.h>

// C/C++ Standard Libraries
#include <cmath>
#include <vector>

// Internal Dependancies
#include "Particle.hpp"
#include "LargeVM.hpp"
#include "Shader.hpp"


#define GD -0.400f
#define KSST 10.00f
#define KSSH 5.00f
#define KSBE 1.00f
#define KDST -0.200f
#define KDSH -0.200f
#define KDBE -0.400f

#define TEX_PATH "4K_UJ_FLAG.png"

class Cloth {
    
public:
    
    enum ParticleModel {
        
        FORCE_MODEL,        // MS Model
        POSITION_MODEL      // PBD Model
    };
    
    enum IntegrationMethod {
        
        F_EULER,
        B_EULER,
        MB_EULER,
        FB_EULER,
        MFB_EULER,
        RK2_EULER,
        RK4_EULER,
        VERLET,
        DEFAULT
    };
    
    enum Preset {
        
        One,
        Two
    };
    
    struct Vertex {
        
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };
    
    struct Spring {
        
        Particle* particleA;
        Particle* particleB;
        
        GLfloat Ks, Kd;     // Mass-Spring vars, Stiffness & Damping constants
        GLfloat K, Kp;      // Position Dynamic vars, ...
        
        GLfloat restLength;
        
        GLuint type;
    };
    
    struct Face {
        
        Particle* particleA;
        Particle* particleB;
        Particle* particleC;
        
        glm::vec3 centrePos;
        glm::vec3 normal;
    };
    
    struct Simplex {
        
        glm::vec3 pointA;
        glm::vec3 pointB;
        glm::vec3 mDifference;
    };
    
    Cloth(GLfloat uWidth, GLfloat uHeight, GLuint nPX, GLuint nPY, Preset preset);
    ~Cloth(void);
    
    // Initialisations
    GLvoid initMesh(GLfloat uWidth, GLfloat uHeight, GLuint nPX, GLuint nPY, Preset preset);
    GLvoid initExtentions(void);
    GLuint loadTexture(const char* path);
    
    GLvoid generateSprings(GLuint x, GLuint y);
    
    GLvoid initSpring(Particle* pA, Particle* pB, GLfloat ks, GLfloat kd, GLuint id);
    GLvoid initFace(Particle* pA, Particle* pB, Particle* pC);
    
    GLvoid setFixedParticle(GLuint x, GLuint y);
    
    GLvoid generateElementIndices(GLuint x, GLuint y, GLuint* e);
    
    inline Particle* getParticle(GLuint x, GLuint y);
    inline GLuint getParticleIndex(GLuint x, GLuint y);
    inline glm::vec3 getPolyNormal(Particle* pA, Particle* pB, Particle* pC);
    inline glm::vec3 verletVelocity(Particle& p, GLfloat dt);
    inline GLfloat randFunc(GLfloat variation);
    
    GLvoid updateFaceNormals(void);
    
    // Internal Properties
    GLvoid setStiffness(GLfloat stS, GLfloat shS, GLfloat beS);
    GLvoid setDamping(GLfloat stD, GLfloat shD, GLfloat beD, GLfloat gD);
    
    GLvoid setWire(GLboolean flag);
    
    // External Forces
    GLvoid addForce(const glm::vec3 direction);
    GLvoid setWind(const glm::vec3 dir, glm::vec3 rVec);
    GLvoid addWindForcesForTriangle(Particle* pA, Particle* pB, Particle* pC);
    
    // Cloth-ObjectMesh Collisions
    GLboolean sphereCollision(const glm::vec3 center, const GLfloat radius);
    GLboolean planarCollision(const GLfloat planePosY);
    
    // Cloth-Cloth Collisions
    GLvoid selfCollision(void);
    GLboolean pointInTriangle(glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 P);
    GLboolean selfCollisionResponse(Face& face, Particle& mQ);
    GLboolean testTriangleIntersect(Particle& particle);
    
    // Physics
    GLvoid stepSimulation(GLfloat dt, IntegrationMethod method, GLboolean dynamicInverse);
    GLvoid computeForces(GLfloat dt, IntegrationMethod method);
    GLvoid constraintsInverse(IntegrationMethod m); // Provot [1995], 'Deformation Constraints in a Mass-Spring Model to Describe Rigid Cloth Behavior'
    
    /// NUMERICAL INTEGRATION SCHEMES - EXPlicit, SYMplectic, IMPlicit
    
    // First-Order Integration Methods
    GLvoid EXP_ForwardEuler(GLfloat dt);
    GLvoid IMP_BackwardEuler(GLfloat dt); // Standard Backward Euler
    GLvoid IMP_ModifiedBackwardEuler(GLfloat dt); // Baraff & Witkin [1998], 'Large steps in cloth simulation'
    GLvoid SYM_ForwardBackwardEuler(GLfloat dt);
    GLvoid SYM_ModifiedForwardBackwardEuler(GLfloat dt); // Desbrun et al. [1999], 'Interactive animation of structured deformable objects', AKA IMEX
    
    // Higher-Order Integration Methods
    GLvoid EXP_Verlet(GLfloat dt);
    GLvoid EXP_RungeKutta2(GLfloat dt);
    GLvoid EXP_RungeKutta4(GLfloat dt);
    
    // Supporting Integration Functions
    GLvoid RK_Stepper(Particle& p, GLfloat dTS, GLfloat fac);
    GLvoid SolveGD(LargeVM<glm::mat3> A, LargeVM<glm::vec3>& x, LargeVM<glm::vec3> b); // Gradient Descent Method
    GLvoid SolveCG(LargeVM<glm::mat3> A, LargeVM<glm::vec3>& x, LargeVM<glm::vec3> b); // Conjugate Gradient Method
    GLvoid SolvePCCG(LargeVM<glm::mat3> A, LargeVM<glm::vec3>& x, LargeVM<glm::vec3> b, LargeVM<glm::vec3> P, LargeVM<glm::vec3> P_inv); // Pre-Conditioned Conjugate Gradient Method
    
    // Draw Vertex/Position Array
    GLvoid render(const Shader* shader);
    
private:
    
    int tmp;
    
    std::vector<Face> faces;
    
    std::vector<Spring> constraints;     // Standard Mass-Spring/Particle-System constraints
    std::vector<Spring> dConstraints;    // Standard Position Based Dynamics distance constraints
    
    std::vector<Particle> particles;
    std::vector<GLuint> vertexIndices;
    
    GLboolean drawWire;
    GLboolean collisionFlag;
    
    glm::vec3 intersectQ;
    
    // Number of Particles X,Y & Total Particles
    const GLuint NumPX, NumPY;
    const GLsizei NumP_XY;
    
    // Preset No.
    const Preset pset;
    
    // Dampening & Stiffness Constants
    GLfloat GlobalDamping;
    GLfloat KsStruct,   KdStruct;
    GLfloat KsShear,    KdShear;
    GLfloat KsBend,     KdBend;
    
    // Physical Constants
    const glm::vec3 Gravity;
    
    // Integration Objects
    std::vector<glm::vec3> sForce;        // Explicit Runge-Kutta 4, Sum of Forces
    std::vector<glm::vec3> sVelocity;     // Explicit Runge-Kutta 4, Sum of Velocities
    
    std::vector<glm::vec3> pForce;        // Symplectic IMEX, Predicted Force
    
    LargeVM<glm::mat3> mMat;         // Implicit Backward Euler, System Mass Matrix
    LargeVM<glm::mat3> kMat;     // Implicit Backward Euler, Jacobian System Stiffness Matrix
    LargeVM<glm::vec3> X, V, F, Vnew;
    
    std::vector<glm::mat3> dForceDX;      // Implicit Modified Backward Euler, ∂F/∂X, Partial Differential (ΔForces / ΔPositions)
    std::vector<glm::mat3> dForceDV;      // Implicit Modified Backward Euler, ∂F/∂V, Partial Differential (ΔForces / ΔVelocitys)
    
    const GLfloat EPS;          // Implicit CG/PCCG constant
    const GLsizei IMAX;         // Implicit CG/PCCG constant
    
    
    const GLsizei solverIterations; // No. of Iterations of constraints satisfaction PBD
    
    // GL Objects
    GLuint TextureID;
    
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    
};

#endif /* Cloth_hpp */
