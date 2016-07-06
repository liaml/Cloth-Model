#include "Cloth.hpp"

#define STRUCTURAL_SPRING 0
#define SHEAR_SPRING 1
#define BEND_SPRING 2

#define I_OFFSET 2 * 3

using namespace std;
using namespace glm;

Cloth::Cloth(GLfloat uWidth, GLfloat uHeight, GLuint nPX, GLuint nPY, enum Preset preset) : NumPX(nPX), NumPY(nPY),NumP_XY((nPX + 1) * (nPY + 1)), Gravity(vec3(0, -0.000981f, 0)), EPS(1/100.0f), IMAX(10), solverIterations(5), pset(preset)
{
    
    faces.clear();
    particles.clear();
    constraints.clear();
    
    sForce.clear(); // Sum Of Forces, Used by Runge-Kutta (RK4) Scheme
    sVelocity.clear(); // Sum Of Velocities, Used by Runge-Kutta (RK4) Scheme
    
    pForce.clear(); // Predicted Forces, Used by IMplicit-Explicit (IMEX) Scheme
    
    dForceDX.clear(); // Partial Differential ∂F/∂X, Used by Modified Backward Euler Scheme
    dForceDV.clear(); // Partial Differential ∂F/∂V, Used by Modified Backward Euler Scheme
    
    particles.resize(NumP_XY);
    vertexIndices.resize((NumPX * NumPY) * I_OFFSET);
    
    // Constraint Stiffness & Damping Settings, Settings Optimised for Euler Integration Schemes
    // TODO: Global Stiffness function
    GlobalDamping = NULL;
    KsStruct = NULL, KsShear = NULL, KsBend = NULL;
    KdStruct = NULL, KdShear = NULL, KdBend = NULL;
    
    drawWire = GL_FALSE;
    collisionFlag = GL_FALSE;
    intersectQ = vec3(0);
    
    VAO = NULL;
    EBO = NULL;
    VBO = NULL;
    
    initMesh(uWidth, uHeight, NumPX, NumPY, pset);
    initExtentions();
}

Cloth::~Cloth(void)
{
}

inline Particle* Cloth::getParticle(GLuint x, GLuint y) {
    return(&particles[getParticleIndex(x, y)]);
}

inline GLuint Cloth::getParticleIndex(GLuint x, GLuint y) {
    return((x + (y * (NumPX + 1))));
}

inline vec3 Cloth::getPolyNormal(Particle* pA, Particle* pB, Particle* pC) {
    return(cross((pB->getPosition() - pA->getPosition()), (pC->getPosition() - pA->getPosition())));
}

inline vec3 Cloth::verletVelocity(Particle& p, GLfloat dt) {
    return((p.getPosition() - p.getLPosition()) / dt);
}

inline GLfloat Cloth::randFunc(GLfloat v) {
    return((-v) + ((GLfloat)rand() / RAND_MAX) * ((v) - (-v)));
}

GLvoid Cloth::setStiffness(GLfloat stS, GLfloat shS, GLfloat beS) {
    
    for (auto c = constraints.begin(); c != constraints.end(); c++) {
        
        if (c->type == STRUCTURAL_SPRING)
            c->Ks = stS;
        
        if (c->type == SHEAR_SPRING)
            c->Ks = shS;
        
        if (c->type == BEND_SPRING) {
            c->Ks = beS;
        }
    }
}

GLvoid Cloth::setWire(GLboolean flag) {
    drawWire = flag;
}

GLvoid Cloth::setDamping(GLfloat stD, GLfloat shD, GLfloat beD, GLfloat gD) {
  
    GlobalDamping = gD;
    
    for (auto c = constraints.begin(); c != constraints.end(); c++) {
        
        if (c->type == STRUCTURAL_SPRING)
            c->Kd = stD;
        
        if (c->type == SHEAR_SPRING)
            c->Kd = shD;
        
        if (c->type == BEND_SPRING) {
            c->Kd = beD;
        }
    }

    KdStruct = stD; KdShear = shD; KdBend = beD;
}

GLvoid Cloth::initExtentions(void) {
    
    mMat.Clear(GL_FALSE);
    kMat.Clear(GL_FALSE);
    X.Clear(GL_FALSE);
    F.Clear(GL_FALSE);
    V.Clear(GL_FALSE);
    Vnew.Clear(GL_FALSE);
    
    X.Resize(NumP_XY);
    F.Resize(NumP_XY);
    V.Resize(NumP_XY);
    Vnew.Resize(NumP_XY);
    
    mMat.Resize(NumP_XY);
    kMat.Resize((GLsizei)constraints.size());
    
    GLfloat tMass = 0.0f;
    
    for (auto p = particles.begin(); p != particles.end(); p++)
        tMass += p->getMass();
    
    mMat = (5.0f) * mMat;
}

GLvoid Cloth::initSpring(Particle* pA, Particle* pB, GLfloat ks, GLfloat kd, GLuint id) {
    
    Spring spring;
    
    spring.particleA = pA;
    spring.particleB = pB;
    spring.Ks = ks;
    spring.Kd = kd;
    spring.type = id;
    
    constraints.push_back(spring);
}

GLvoid Cloth::initFace(Particle* pA, Particle* pB, Particle* pC) {
    
    Face face;
    
    face.particleA = pA;
    face.particleB = pB;
    face.particleC = pC;
    
    faces.push_back(face);
}

GLvoid Cloth::initMesh(GLfloat uWidth, GLfloat uHeight, GLuint NumPX, GLuint NumPY, enum Preset preset) {
    
    GLfloat unitDistX = uWidth / NumPX;
    GLfloat unitDistY = uHeight / NumPY;

    // Create 2D mesh, aligned centre
    for (GLsizei j = 0; j <= NumPY; j++) {
        for (GLsizei i = 0; i <= NumPX; i++) {
            
            // Initial mesh particle positions, with variance added to the true position, this allows the cloth to naturally acquire out of plane deformations
            
            vec3 particlePos;
            
            switch (preset) {
                
                case One:
                    
                    particlePos = vec3(((GLfloat(i) * unitDistX) - (uWidth / 2)) + randFunc(unitDistX / 100),
                                            (uHeight - (GLfloat(j) * unitDistY)) + randFunc(unitDistY / 100),
                                            randFunc(((unitDistX / 100) + (unitDistY / 100)) / 2.0f));

                    break;
                  
                case Two:
                    
                    particlePos = vec3(((GLfloat(i) * unitDistX) - (uWidth / 2)) + randFunc(unitDistX / 100),
                                            randFunc(((unitDistX / 100) + (unitDistY / 100)) / 2.0f) + uHeight,
                                            (uHeight - (GLfloat(j) * unitDistY)) + randFunc(unitDistY / 100) - (uHeight/2.0f));
                    break;
                    
                default:
                    break;
            }
        
            
            ///CLOTH PRESET 2
            
            /**/

            
            particles[i + (j * (NumPX + 1))] = Particle(particlePos);
            particles[i + (j * (NumPX + 1))].setIndex(i + (j * (NumPX + 1)));
            
            generateSprings(i, j);
            
            if (i < NumPX && j < NumPY) {
                
                GLuint* element = &vertexIndices[(getParticleIndex(i, j) * I_OFFSET) - (j * I_OFFSET)];
                
                generateElementIndices(i, j, element);
                
                initFace(getParticle(i + 1, j), getParticle(i, j), getParticle(i, j + 1));
                initFace(getParticle(i + 1, j + 1), getParticle(i + 1, j), getParticle(i, j + 1));
            }
        }
    }
    
    // Set constraints/springs rest length values
    for (auto c = constraints.begin(); c != constraints.end(); c++)
        c->restLength = distance(c->particleA->getPosition(), c->particleB->getPosition());
    
    // Set faces centre and normal values
    for (auto f = faces.begin(); f != faces.end(); f++) {
        f->centrePos = (f->particleA->getPosition() + f->particleB->getPosition() + f->particleC->getPosition()) / 3.0f;
        f->normal = normalize(getPolyNormal(f->particleA, f->particleB, f->particleC));
    }
    
    // Set default fixed particles
   // for (GLsizei i = 0; i <= NumPX; i += (NumPX/1)) {
        getParticle(0, 0)->setFlag(GL_TRUE);
        getParticle(NumPX, 0)->setFlag(GL_TRUE);
    //}
    


}

GLvoid Cloth::generateElementIndices(GLuint x, GLuint y, GLuint* e){
    
    GLuint e0 = getParticleIndex(x, y);
    GLuint e1 = getParticleIndex((x + 1), y);
    GLuint e2 = getParticleIndex(x, (y + 1));
    GLuint e3 = getParticleIndex((x + 1), (y + 1));
    
    *e++ = e0; *e++ = e2; *e++ = e1;
    *e++ = e1; *e++ = e2; *e++ = e3;
}

GLvoid Cloth::generateSprings(GLuint x, GLuint y) {
    
    // Structural Spring
    if (x < NumPX) {
        initSpring(getParticle(x, y), getParticle((x + 1), y),
                   KsStruct, KdStruct, STRUCTURAL_SPRING);
    }
    if (y < NumPY) {
        initSpring(getParticle(x, y), getParticle(x, (y + 1)),
                   KsStruct, KdStruct, STRUCTURAL_SPRING);
    }
    
    // Shear Spring
    if (x < NumPX && y < NumPY) {
        initSpring(getParticle(x, y), getParticle((x + 1), (y + 1)),
                   KsShear, KdShear, SHEAR_SPRING);
    }
    if (x < NumPX && y < NumPY) {
        initSpring(getParticle((x + 1), y), getParticle(x, (y + 1)),
                   KsShear, KdShear, SHEAR_SPRING);
    }
    
    // TODO: Fix Bend Springs, need ability to switch between structural bend, stretch bend, and both (PBD) spring setups

    
    if (x < NumPX - 1) {
        initSpring(getParticle(x, y), getParticle((x + 2), y),
                   KsBend, KdBend, BEND_SPRING);
    }
    if (y < NumPX - 1) {
        initSpring(getParticle(x, y), getParticle(x, (y + 2)),
                   KsBend, KdBend, BEND_SPRING);
    }
    /*if (x < (NumPX - 1) && y < (NumPY - 1)) {
        initSpring(getParticle(x, y), getParticle((x + 2), (y + 2)),
                   KsBend, KdBend, BEND_SPRING);
    }
    if (x < (NumPX - 1) && y < (NumPY - 1)) {
        initSpring(getParticle((x + 2), y), getParticle(x, (y + 2)),
                   KsBend, KdBend, BEND_SPRING);
    }*/
    
}

// Function is to computationally complex, TODO: Optimisation Task, Incoporate per particle 'Compute Forces', per particle 'Integration', and per constraint 'Resolve Forces' iterations into the a single per constraint loop iteration function. 01/03/16

GLvoid Cloth::stepSimulation(GLfloat dt, IntegrationMethod m, GLboolean dyIn) {
    
    computeForces(dt, m);
    
    // Position Displacement Integrarion Schemes
    switch (m) {
            
        case F_EULER:
            // TODO, RFT, Forward Euler Method
            EXP_ForwardEuler(dt);
            break;
            
        case B_EULER:
            // TODO: EFT, Implicit Backward-Euler (BDF) Conjugate Gradient Method, More Info wiki/Euler_methods
            IMP_BackwardEuler(dt);
            break;
            
        case MB_EULER:
            // TODO: EFT, Implicit Backward-Euler (BDF) Pre-Conditioned Conjugate Gradient Method, from Baraff & Witkin [1998]
            IMP_ModifiedBackwardEuler(dt);
            dForceDV.clear();
            dForceDX.clear();
            break;
            
        case FB_EULER:
            // TODO: EFT, Symplectic Euler Method, wiki/Symplectic_Euler_method
            SYM_ForwardBackwardEuler(dt);
            break;
            
        case MFB_EULER:
            // TODO: EFT, IMplicit-EXplicit Modified Euler (Newton-Krylov Solver) Method from Desbrun et al. [1999/ref2]
            SYM_ModifiedForwardBackwardEuler(dt);
            pForce.clear();
            break;
            
        case RK2_EULER:
            // TODO: EFT, Explicit Midpoint-Euler/RK2 Method, More Info @ wiki/Runge-Kutta_methods
            EXP_RungeKutta2(dt);
            break;
            
        case RK4_EULER:
            // TODO: RFT, Explicit Runge-Kutta 4th Order Method, More Info @ wiki/Runge-Kutta_methods
            EXP_RungeKutta4(dt);
            sForce.clear();
            sVelocity.clear();
            break;
            
        case VERLET:
            // TODO: RFT, Verlet Method
            EXP_Verlet(dt);
            break;
            
        case DEFAULT:
            break;
    }
    
    if (dyIn) 
        constraintsInverse(m);
    
    // reset forces for all particles
    for (auto p = particles.begin(); p != particles.end(); p++)
        p->resetForce();
     
}

GLvoid Cloth::computeForces(GLfloat dt = NULL, IntegrationMethod m = DEFAULT) {
    
    mat3 I = mat3(1);
    
    // Compute Forces Per Particle
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        if (m == VERLET)
            p->setVelocity(verletVelocity(*p, dt));
        
        // + G-force
        if (p->getFlag()) {
            
            if (m == VERLET)
                p->addForce(Gravity * p->getMass());
            else
                p->addForce(Gravity);
        }
        
        // + Force Due To Dampening
        if (m == VERLET)
            p->addForce(GlobalDamping * verletVelocity(*p, dt)); // Position Based
        else
            p->addForce(GlobalDamping * p->getVelocity()); // Force Based
    }

    GLsizei ittInd = 0;
    
    // Resolve Forces Per Constraint
    for (auto c = constraints.begin(); c != constraints.end(); c++) {
        
        vec3 deltaP = c->particleA->getPosition() - c->particleB->getPosition();
        vec3 deltaV = c->particleA->getVelocity() - c->particleB->getVelocity();
        
        GLfloat sLength = glm::length(deltaP);
        
        // TODO: Calculate Derivative Force, Baraff & Witkin [1998/ref3], Section 4.1 pp. 46
        if (m == MB_EULER) {
            
            vec3 deltaP2 = deltaP * deltaP;
            vec3 pdC_pdX = deltaP / sLength;
            
            GLfloat C = sLength - c->restLength;
            GLfloat C_DOT = dot(c->particleA->getVelocity(), -pdC_pdX) + dot(c->particleB->getVelocity(), pdC_pdX);
            
            mat3 pdF_pdX, pdF_pdV;
            mat3 pd2C_pdX2[2][2];
            
            pd2C_pdX2[0][0][0][0] = (-C * deltaP2.x) + C;
            pd2C_pdX2[0][0][1][1] = (-C * deltaP2.y) + C;
            pd2C_pdX2[0][0][2][2] = (-C * deltaP2.z) + C;
            
            pd2C_pdX2[0][1][0][0] = (C * deltaP2.x) - C;
            pd2C_pdX2[0][1][1][1] = (C * deltaP2.y) - C;
            pd2C_pdX2[0][1][2][2] = (C * deltaP2.z) - C;
            
            pd2C_pdX2[1][0] = pd2C_pdX2[0][1];
            pd2C_pdX2[1][1] = pd2C_pdX2[0][0];
            
            mat3 pdX1 = outerProduct(pdC_pdX, pdC_pdX);
            mat3 pdX2 = outerProduct(pdC_pdX, -pdC_pdX);
            mat3 pdX3 = outerProduct(-pdC_pdX, -pdC_pdX);
            
            pdF_pdX += - (c->Ks * (pdX1 + (pd2C_pdX2[0][0] * C))) - (c->Kd * (pd2C_pdX2[0][0] * C_DOT));
            pdF_pdX += - (c->Ks * (pdX2 + (pd2C_pdX2[0][1] * C))) - (c->Kd * (pd2C_pdX2[0][1] * C_DOT));
            pdF_pdX += - (c->Ks * (pdX3 + (pd2C_pdX2[1][1] * C))) - (c->Kd * (pd2C_pdX2[1][1] * C_DOT)); // Problem potentially here
            
            pdF_pdV += - c->Kd * pdX1;
            pdF_pdV += - c->Kd * pdX2;
            pdF_pdV += - c->Kd * pdX3;
            
            dForceDX.push_back(pdF_pdX);
            dForceDV.push_back(pdF_pdV);
        }
        
        if (B_EULER) {
            
            // Constraints Stiffness Jacobian
            kMat[ittInd] = c->Ks * ( ( (c->restLength / sLength) * (I - outerProduct(deltaP, deltaP)/pow(sLength, sLength))) - I);
            ittInd++;
        }
        
        GLfloat LHS = -(c->Ks) * (sLength - c->restLength);
        GLfloat RHS = c->Kd * (dot(deltaV, deltaP) / sLength);
        
        vec3 ResultantForce = (LHS + RHS) * glm::normalize(deltaP);
        
            c->particleA->addForce(ResultantForce);
            c->particleB->addForce(-ResultantForce);
    }
    
}

// Extended Feature, 'Dynamic Inverse' on constraints, Provot [1995/ref1], 'Deformation constraints in a mass-spring model to describe rigid cloth behavior' Section 5

GLvoid Cloth::constraintsInverse(IntegrationMethod m = DEFAULT) {
    
    for (auto c = constraints.begin(); c != constraints.end(); c++) {
        
        // Implementation not currently working correctly - could be an inherited issue, 11/02/2016
        
        vec3 deltaP = c->particleA->getPosition() - c->particleB->getPosition();
        
        GLfloat cLength = length(deltaP); // length of constraint
        
        if (cLength > c->restLength) {
            
            cLength -= c->restLength; // length difference between constraint and its rest length
            cLength /= 2.0f;
            deltaP = glm::normalize(deltaP); // unit normal
            deltaP *= cLength;
            
            if (!(c->particleA->getFlag())) {
                
                if (m == VERLET)
                    c->particleB->offsetPosition(deltaP);
                else
                    c->particleB->offsetVelocity(deltaP);
            } else if (!(c->particleB->getFlag())) {
                
                if (m == VERLET)
                    c->particleA->offsetPosition(-deltaP);
                else
                    c->particleA->offsetVelocity(-deltaP);
            } else {
                
                if (m == VERLET) {
                    
                    c->particleA->offsetPosition(-deltaP);
                    c->particleB->offsetPosition(deltaP);
                } else {
                    
                    c->particleA->offsetVelocity(-deltaP);
                    c->particleB->offsetVelocity(deltaP);
                }
            }
        }
    }
}

/// INTEGRATION SCHEMES

// Jacobsen [2001/ref4], The precursor to position based dynamics formalised in Müller et al. [2006]
GLvoid Cloth::EXP_Verlet(GLfloat dt) {
    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        vec3 X = p->getPosition();
        vec3 Xlast = p->getLPosition();
        vec3 A = p->getAcceleration();
        
        X = X + (X - Xlast) + (A * powf(dt, 2));
        
        p->setNewPosition(X);
    }
}

GLvoid Cloth::EXP_ForwardEuler(GLfloat dt) {
    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        vec3 V = p->getVelocity();
        vec3 Vlast = p->getLVelocity();
        vec3 X = p->getPosition();
        vec3 A = p->getAcceleration();
        
        V = V + (A * dt);
        X = X + (Vlast * dt);
        
        p->setNewVelocity(V);
        p->setNewPosition(X);
    }
}

GLvoid Cloth::EXP_RungeKutta2(GLfloat dt) {
    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        vec3 V = p->getVelocity();
        vec3 Vlast = p->getLVelocity();
        vec3 X = p->getPosition();
        vec3 A = p->getAcceleration();
        
        V = V + ((dt/2.0f) * A);
        X = X + dt * Vlast;
        
        p->setNewVelocity(V);
        p->setNewPosition(X);
    }
    
    computeForces();
    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        vec3 V = p->getVelocity();
        vec3 Vlast = p->getLVelocity();
        vec3 X = p->getPosition();
        vec3 A = p->getAcceleration();
        
        V = V + (dt * A);
        X = X + dt * Vlast;
        
        p->setNewVelocity(V);
        p->setNewPosition(X);
    }
}
// TODO: Optimise RK4 Implementation, Method should be fast however it is the slowest of all integration schemes
GLvoid Cloth::EXP_RungeKutta4(GLfloat dt) {
    
    // Initialisation
    for (auto p = particles.begin(); p != particles.end(); p++)
        RK_Stepper(*p, (dt / 2.0f), (1 / 6.0f));
    
    computeForces();
    
    for (auto p = particles.begin(); p != particles.end(); p++)
        RK_Stepper(*p, (dt / 2.0f), (1 / 3.0f));
    
    computeForces();
    
    for (auto p = particles.begin(); p != particles.end(); p++)
        RK_Stepper(*p, dt, (1 / 3.0f));
    
    computeForces();
    
    for (auto p = particles.begin(); p != particles.end(); p++)
        RK_Stepper(*p, dt, (1 / 6.0f));
    
    // Integration
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        vec3 V = p->getVelocity();
        vec3 X = p->getPosition();
        vec3 Fs = sForce[p->getIndex()];
        vec3 Vs = sVelocity[p->getIndex()];
        
        V = V + Fs;
        X = X + Vs;
        
        p->setNewVelocity(V);
        p->setNewPosition(X);
    }
}

GLvoid Cloth::SYM_ForwardBackwardEuler(GLfloat dt) { // Symplectic Euler
    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        vec3 V = p->getVelocity();
        vec3 X = p->getPosition();
        vec3 A = p->getAcceleration();
        
        V = V + (A * dt);
        X = X + (V * dt);
        
        p->setNewVelocity(V);
        p->setNewPosition(X);
    }
}

// Desbrun et al. [1999/ref2], 'Interactive Animation Of Structured Deformable Objects'
GLvoid Cloth::SYM_ModifiedForwardBackwardEuler(GLfloat dt) { // AKA IMplicit-EXplicit (IMEX) Scheme
 
    vec3 Xg = vec3(0); // Centre of Gravity
    vec3 dTor = vec3(0); // Global Torque
    vec3 Fc = vec3(0); // Corrected Force
    
    // Prediction
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        GLfloat m = p->getMass();
        
        vec3 Fp = vec3(0);
        vec3 F = p->getForce();
        vec3 X = p->getPosition();
        
        mat3 I = mat3(1); // Identity/Unit Matrix
        mat3 H = mat3(-1, 1, 0,
                      1,-2, 1,
                      0, 1,-1); // Hessian Matrix
        
        mat3 W = inverse(I - H * (powf(dt, 2) / m)); // Filter
        
        Fp = Fp + F * W; // Predicted Force
        Xg = Xg + X;
        
        pForce.push_back(Fp);
    }
    
    Xg /= NumP_XY;
    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        vec3 Fp = pForce[p->getIndex()];
        vec3 X = p->getPosition();
        
        dTor = dTor + cross(Fp, X);
    }
    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        GLfloat m = p->getMass();
        
        vec3 V = p->getVelocity();
        vec3 X = p->getPosition();
        vec3 F = p->getForce();
        vec3 Fp = pForce[p->getIndex()];
        
        V = V + ((((F + Fp) * dt) / m));
        X = X + (V * dt);
        
        p->setNewVelocity(V);
        p->setNewPosition(X);
    }
    
    // Correction
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        GLfloat m = p->getMass();
        vec3 X = p->getPosition();
        
        Fc = cross((Xg - X), dTor) * dt; // Corrected Force
        X = X + Fc * (powf(dt, 2) / m);
        
        p->setNewPosition(X);
    }
}

// Standard Implicit Euler
GLvoid Cloth::IMP_BackwardEuler(GLfloat dt) {
    
    // TODO: Extended Task, integrate LargeVM and Particle classes more seamlessly
    
    GLfloat tMass = 0;
    mat3 I = mat3(1); // Identity Matrix
    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        X[p->getIndex()] = p->getPosition();
        F[p->getIndex()] = p->getForce();
        V[p->getIndex()] = p->getVelocity();
        
        tMass += p->getMass();
    }
    
    //mMat = (tMass / NumP_XY) * mMat;
    
    LargeVM<mat3> A = mMat - (pow(dt, 2)*kMat);
    LargeVM<vec3> b = (mMat* V) + (dt * F);
    
    //SolveCG(A, Vnew, b);
    SolveGD(A, Vnew, b);

    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        GLsizei ind = p->getIndex();
        
        X[ind] += (dt * Vnew[ind]);
        V[ind] = Vnew[ind];
        
        p->setNewPosition(X[ind]);
        p->setNewVelocity(V[ind]);
    }
}

GLvoid Cloth::IMP_ModifiedBackwardEuler(GLfloat dt) {
    
    // TODO: Baraff & Witkin [1998] Implicit Integration Scheme
}

// Supporting Integration Functions
GLvoid Cloth::RK_Stepper(Particle& p, GLfloat dTS, GLfloat fc) {
    
    vec3 Fs, Vs;
    vec3 V = p.getVelocity();
    vec3 A = p.getAcceleration();
    
    Fs = A * dTS;
    Vs = V * dTS * fc;
    
    sForce.push_back(Fs);
    sVelocity.push_back(Vs);
}

GLvoid Cloth::SolveGD(LargeVM<mat3> A, LargeVM<vec3>& x, LargeVM<vec3> b) {
    
    // NOTE, operators are overloaded by LargeVM
    
    LargeVM<vec3> r = b - (A * x);
    LargeVM<vec3> q;
    
    GLfloat alpha;
   
    GLfloat del = dot(r, r);
    GLfloat del0 = del;
    
    GLfloat eTol = std::pow(1*10, -10); // Error tolerance
    GLsizei iterMax = 10; // maximum iterations
    GLsizei i = 0;
    
    while ((del > (std::pow(eTol, 2) * del0)) && (i < iterMax)) {
        
        q = A * r;
        
        alpha = del / dot(r, q);
        
        x = x + alpha * r;
        r = r - alpha * q;
        
        del = dot(r, r);
        
        i++;
    }
    
    
}

GLvoid Cloth::SolveCG(LargeVM<mat3> A, LargeVM<vec3>& x, LargeVM<vec3> b) {
    
    // NOTE, operators are overloaded by LargeVM
    
    LargeVM<vec3> r = b - (A * x);
    LargeVM<vec3> d = r;
    LargeVM<vec3> q;
    
    GLfloat alpha, beta;
    
    GLfloat delOld;
    GLfloat delNew = dot(r, r); // 'dot' here is not directly the same as glm::dot, but a function of LargeVM
    GLfloat del0 = delNew;
    
    GLfloat eTol = std::pow(1*10, -10); // Error tolerance
    GLsizei iterMax = 10; // maximum iterations
    GLsizei i = 0;
    
    while ((delNew > (std::pow(eTol, 2) * del0)) && (i < iterMax)) { // iterate until (Ax = b is satisfied) or (iterMax is met)
        
        q = A * d;
        
        alpha = delNew / dot(d, q);
        
        x = x + alpha * d;
        r = r - alpha * q;
        
        delOld = delNew;
        delNew = dot(r, r);
        beta = delNew / delOld;
        
        d = r + (beta * d);
        
        i++;
    }
}

GLvoid Cloth::SolvePCCG(LargeVM<mat3> A, LargeVM<vec3>& x, LargeVM<vec3> b, LargeVM<vec3> P, LargeVM<vec3> P_inv) {
    
    // Baraff & Witkin [1998]
}

// TODO: Required Feature, Cloth-Object Collisions
GLboolean Cloth::sphereCollision(const vec3 center, const GLfloat radius) {
    
    GLboolean cFlag = GL_FALSE;
    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        vec3 delta = p->getPosition() - center; // vector from point to cloth vertices
        GLfloat dist = glm::length(delta);
        
        if (dist < radius) { // intersection
            
            cFlag = GL_TRUE;
            
            p->setCol(GL_TRUE);
            p->offsetPosition(normalize(delta) * (radius - dist));
            
            
            p->setVelocity(vec3(0));
        }
    }
    
    return(cFlag);
}

GLboolean Cloth::planarCollision(GLfloat plPosY) {
    
    GLboolean cFlag = GL_FALSE;
    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        if (p->getPosition().y <= plPosY) {
            
            cFlag = GL_TRUE;
            
            vec3 buffer = p->getPosition();
            buffer.y = plPosY;
            
            p->setPosition(buffer);
        }
    }
    
    return(cFlag);
}

// TODO: Extended Feature, Cloth-Cloth Collisions

GLvoid Cloth::selfCollision(void) {
    
    GLboolean flag = GL_TRUE;
    
    for (auto p = particles.begin(); p != particles.end(); p++) {
        
        for (auto f = faces.begin(); f != faces.end(); f++) {
            
            if ((f->particleA->getIndex() != p->getIndex()) &&
                (f->particleB->getIndex() != p->getIndex()) &&
                (f->particleC->getIndex() != p->getIndex())) {
                
                if (pointInTriangle(f->particleA->getPosition(), f->particleB->getPosition(), f->particleC->getPosition(), p->getPosition())) {
                    
                    selfCollisionResponse(*f, *p);
                    flag = GL_FALSE;
                }
            }
        }
        
        if (flag) {
            
            testTriangleIntersect(*p);
        }
    }
}

GLboolean Cloth::pointInTriangle(vec3 A, vec3 B, vec3 C, vec3 P) {
    
    // Prepare barycentric values
    vec3 u = B - A;
    vec3 v = C - A;
    vec3 w = P - A;
    
    vec3 vCrossW = cross(v, w);
    vec3 vCrossU = cross(v, u);
    
    // Test sign of r
    if (dot(vCrossW, vCrossU) < 0)
        return(GL_FALSE);
    
    vec3 uCrossW = cross(u, w);
    vec3 uCrossV = cross(u, v);
    
    // Test sign of t
    if (dot(uCrossW, uCrossV) < 0)
        return(GL_FALSE);
    
    GLfloat denominator = length(uCrossV);
    
    GLfloat r = length(vCrossW) / denominator;
    GLfloat t = length(uCrossW) / denominator;
    
    return(r + t <= 1);
}

GLboolean Cloth::selfCollisionResponse(Face& face, Particle& mQ) {
    
    vec3 Q = mQ.getPosition();
    vec3 pA = face.particleA->getPosition();
    vec3 pB = face.particleB->getPosition();
    vec3 pC = face.particleC->getPosition();
    
    Q = Q - pA;
    pB = pB - pA;
    pC = pC - pA;
    pA = vec3(0);
    
    vec3 normal(cross(pB, pC));
    GLfloat lengthAB = glm::length(normal);
    normal = normalize(normal);
    
    GLfloat value = dot(Q, normal) - 0.001f;
    
    if (value > 0)
        return(GL_TRUE);
    
    vec3 dcQ = normal;
    vec3 dcpB = (cross(pC, Q) + cross(normal, pC) * dot(normal, Q)) / lengthAB;
    vec3 dcpC = -(cross(pB, Q) + cross(normal, pB) * dot(normal, Q)) / lengthAB;
    vec3 dcpA = -dcQ - dcpB - dcpC;
    
    GLfloat wQ = 1.0f;
    GLfloat wA = face.particleA->getMass();
    GLfloat wB = face.particleB->getMass();
    GLfloat wC = face.particleC->getMass();
    
    GLfloat denominator = (wA * dot(dcpA, dcpA)) +
                          (wB * dot(dcpB, dcpB)) +
                          (wC * dot(dcpC, dcpC)) +
                          (wQ * dot(dcQ, dcQ));
    //assert(denominator < 0.0001f);
    
    GLfloat S = value / denominator;
    
    vec3 dQ = -wQ * S * dcQ;
    vec3 dpA = -wA * S * dcpA;
    vec3 dpB = -wB * S * dcpB;
    vec3 dpC = -wC * S * dcpC;
    
    mQ.getPosition() += dQ;
    face.particleA->getPosition() += dpA;
    face.particleB->getPosition() += dpB;
    face.particleC->getPosition() += dpC;
    
    return(GL_FALSE);
}

GLboolean Cloth::testTriangleIntersect(Particle& p) {
 
    GLboolean testResult = GL_FALSE;
    
    for (auto f = faces.begin(); f != faces.end(); f++) {
        
        if ((f->particleA->getIndex() != p.getIndex()) &&
            (f->particleB->getIndex() != p.getIndex()) &&
            (f->particleC->getIndex() != p.getIndex())) {
            
            Face currentFace = *f;
            vec3 n = normalize(f->normal);
            
            vec3 rayDirection = p.getPosition() - p.getLPosition();
            GLfloat nDotD = dot(rayDirection, n);
            
            if ((abs(nDotD) <= 0.100) || (nDotD == 0))
                return(GL_FALSE);
            else {
                
                GLfloat d = dot(n, f->particleA->getPosition());
                GLfloat t = (d - dot(n, p.getLPosition())) / nDotD;
                
                intersectQ = p.getLPosition() + (t * rayDirection);
                
                if ((intersectQ.x >= glm::max(p.getPosition().x, p.getLPosition().x)) ||
                    (intersectQ.y >= glm::max(p.getPosition().y, p.getLPosition().y)) ||
                    (intersectQ.z >= glm::max(p.getPosition().z, p.getLPosition().z)) ||
                    (intersectQ.x <= glm::min(p.getPosition().x, p.getLPosition().x)) ||
                    (intersectQ.y <= glm::min(p.getPosition().y, p.getLPosition().y)) ||
                    (intersectQ.z <= glm::min(p.getPosition().z, p.getLPosition().z))) {
                        
                        return(GL_FALSE);
                }
                
                if (pointInTriangle(f->particleA->getPosition(), f->particleB->getPosition(), f->particleC->getPosition(), intersectQ)) {
                    
                    p.setPosition(p.getLPosition());
                    return(GL_TRUE);
                }
            }
        }
    }
    return(GL_FALSE);
}

// TODO: Required Feature, Basic Wind
GLvoid Cloth::setWind(const vec3 dir, vec3 rVec) {
    
    for (auto f = faces.begin(); f != faces.end(); f++) {
        
        vec3 normal = getPolyNormal(f->particleA, f->particleB, f->particleC);
        vec3 force = normal * dot(normalize(normal), dir + vec3(randFunc(rVec.x),
                                                                randFunc(rVec.y),
                                                                randFunc(rVec.z)));
        
        f->particleA->addForce(force);
        f->particleB->addForce(force);
        f->particleC->addForce(force);
    }
}

GLuint Cloth::loadTexture(const char* path) {
    
    /*
    GLint width, height;
    
    glGenTextures(1, &TextureID);
    unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
    
    // Assign Texture to ID
    glBindTexture(GL_TEXTURE_2D, TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    
    return(TextureID);
    */
    
    return(0);
}

GLvoid Cloth::render(const Shader* shader) {
    
    glPolygonMode(GL_FRONT_AND_BACK, (drawWire ? GL_LINE : GL_FILL));
    
    GLuint texture;
    
    for (auto p = particles.begin(); p != particles.end(); p++)
        p->resetNormal();
    
    // Vertex Normals
    for (auto f = faces.begin(); f != faces.end(); f++) {
        
        vec3 newNormal = getPolyNormal(f->particleA, f->particleB, f->particleC);
        
        f->particleA->addToNormal(newNormal);
        f->particleB->addToNormal(newNormal);
        f->particleC->addToNormal(newNormal);
    }
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    GLuint positionAttributeLocation = glGetAttribLocation(shader->ProgramID, "i_position");
    glEnableVertexAttribArray(positionAttributeLocation);
    glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0);
    
    // TODO: Extended Feature, Blinn-Phong Lighting
    
    GLuint normalAttributeLocation = glGetAttribLocation(shader->ProgramID , "i_normal");
    glEnableVertexAttribArray(normalAttributeLocation);
    glVertexAttribPointer(normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)sizeof(vec3));
    
    GLuint uvAttributeLocation = glGetAttribLocation(shader->ProgramID, "i_uv");
    glEnableVertexAttribArray(uvAttributeLocation);
    glVertexAttribPointer(uvAttributeLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)(sizeof(vec3) + sizeof(vec3)));
    
    // TODO: Extended Feature, Texturing
    
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(GLuint), &vertexIndices[0], GL_STATIC_DRAW);
    
    //texture = loadTexture(TEX_PATH);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader->ProgramID, "i_texture"), 0);
    
    vector<Vertex> vertexData;
    
    for (GLsizei j = 0; j <= NumPY; j++) {
        for (GLsizei i = 0; i <= NumPX; i++) {
            
            Vertex v;
            v.position = getParticle(i, j)->getPosition();
            v.normal = getParticle(i, j)->getNormal();
            v.uv = vec2((i / NumPX), (j / NumPY));
            vertexData.push_back(v);
        }
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), value_ptr(vertexData[0].position), GL_STATIC_DRAW);
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)vertexIndices.size(), GL_UNSIGNED_INT, 0);

    // Deallocate Memory
    glDeleteTextures(1, &texture);
    
    glDisableVertexAttribArray(positionAttributeLocation);
    glDisableVertexAttribArray(normalAttributeLocation);
    glDisableVertexAttribArray(uvAttributeLocation);
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);    
}