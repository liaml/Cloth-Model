/// 08341 | FINAL YEAR DEVELOPMENT PROJECT

/// THIS CODE SUPPORTS THE 08341 FYDP FINAL REPORT, LEES [2016], 'CLOTH MODELLING', SUBMITTED IN PARTIAL FULFILMENT OF THE REQUIREMENTS FOR THE DEGREE OF 'BACHELOR OF SCIENCE' IN THE FACULTY OF UNDERGRADUATE STUDIES (DEPT. OF COMPUTER SCIENCE, THE UNIVERSITY OF HULL)

/// TESTED ON MAC OS X 10.11

/// REFERENCES
/// [REF1] Provot, X. 1995, 'Deformation constraints in a mass-spring model to describe rigid cloth behavior'. https://graphics.stanford.edu/courses/cs468-02-winter/Papers/Rigidcloth.pdf
/// [REF2] Desbrun, M., et al. 1999, 'Interactive animation of structured deformable objects'. http://multires.caltech.edu/pubs/GI99.pdf
/// [REF3] Baraff, D., & Witkin, A. 1998, 'Large steps in cloth simulation'. http://cs.cmu.edu/~baraff/papers/sig98.pdf
/// [REF4] Jackobsen, T. 2001, 'Advanced Character Physics'. http://web.archive.org/web/20100111035201/http://www.teknikus.dk/tj/gdc2001.htm
/// [REF5] Müller, M, 2007, 'Position Based Dynamics'. http://matthias-mueller-fischer.ch/publications/posBasedDyn.pdf

/// COPYRIGHT © LIAM CHRISTOPHER LEES, 2016

// OpenGL Extensions Wrangler Library
#include <GL/glew.h>

// OpenGL Window & Peripherals Handling Library
#include <GLFW/glfw3.h>

// AntTweakBar GUI Library
#include <ATB/AntTweakBar.h>

// OpenGL Mathematics Library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

// C/C++ Libraries
#include <iostream>

// Internal Dependancies
#include "Shader.hpp"
#include "Cloth.hpp"
#include "ObjectMesh.hpp"
#include "TrackBall.hpp"

// PP Macros

// Normal Mapper Code EXPERIMENTAL
#define NORMAL_VERTEX_SHADER "vShader.glsl"
#define GEOMETRY_SHADER_PATH "gShader.glsl"
#define NORMAL_FRAGMENT_SHADER "fShader.glsl"

// Physically Based Lighter (Blinn-Phong) Energy Model
#define LIGHTING_VERTEX_SHADER "vLightingShader.glsl"
#define LIGHTING_FRAGMENT_SHADER "fLightingShader.glsl"


#define APP_NAME "Copyright © Liam Christopher Lees, 2016"

#define SPHERE_DIMENSIONS 64, 32, 0.20f
#define PLANE_DIMENSIONS 10, 0.25f

#define BG_FT_DARK 0.137254902f, 0.1568627451f, 0.1764705882f
#define BG_FT_LIGHT 0.7294117647f, 0.7607843137f, 0.8f
#define BG_FIGURE_CAPTURE 1,1,1

/// GLOBALS

struct LightSource {
    
    vec3 ambientColour;
    vec3 diffuseColour;
    vec3 specularColour;
    
    vec3 position;
};

struct Material {
    
    vec3 ambientColour;
    vec3 diffuseColour;
    vec3 specularColour;
    GLfloat specularShininess;
};

Shader* mainShader;
Shader* subShader;
Cloth* clothObject;
ObjectMesh* sphereObject;
ObjectMesh* plane;

LightSource light;
Material material;

GLFWwindow* gWindow;
TwBar* tMain; // Cloth GUI Bar
TwBar* tSub;  // Subsidary GUI Bar
TwBar* tScene; // Scene Management Bar

static vec3 backgroundC = vec3(BG_FT_LIGHT);

static const GLuint WIN_DIMENSION_WIDTH = 1440, WIN_DIMENSION_HEIGHT = 800;
static GLint pixWidth, pixHeight;
static GLint corWidth, corHeight;

static GLfloat UNIT_WIDTH = 1, UNIT_HEIGHT = 1;
static GLuint NxN = 50;
static GLuint NO_PARTICLES_X = NxN, NO_PARICLES_Y = NxN, Nc = NO_PARTICLES_X * NO_PARICLES_Y;

static GLfloat timestep = 0.0100f;

static Cloth::IntegrationMethod integration = Cloth::MFB_EULER;
static Cloth::Preset preset = Cloth::One;

// TODO: Fix Hacky Code Below
static GLfloat globalDamping = GD;
static GLfloat stretchStiffness = KSST, shearStiffness = KSSH, bendStiffness = KSBE;
static GLfloat stretchDamping = KDST, shearDamping = KDSH, bendDamping = KDBE;

static GLboolean normals = GL_FALSE;
static GLfloat nLength = 0.075f;
static GLfloat nVibrance = 0.5f;

static GLboolean pausePlay = GL_TRUE;

static GLboolean forces = GL_FALSE;
static GLfloat fVibrance = 0.5f;

static GLboolean velocities = GL_FALSE;
static GLfloat vVibrance = 0.5f;

static GLboolean inverseDy = GL_TRUE;
static GLboolean selfCol = GL_FALSE;

static GLboolean planarCol = GL_TRUE;
static GLboolean sphereCol = GL_FALSE;

static GLboolean drawPlane = GL_FALSE;
static GLboolean drawSphere = GL_FALSE;
static GLboolean drawCloth = GL_TRUE;

static GLboolean drawW = GL_TRUE;

static GLboolean wind = GL_FALSE;
static vec3 windDirection = vec3(-0.91,-0.17,0.38);
static GLfloat wIntensity = 3.6f;
static vec3 variance = vec3(0);
static GLfloat ppTurbulance;

static GLfloat FoV = 28.0f;

static vec3 LightDirection = vec3(0,0,1);

static GLboolean clothReInit = GL_FALSE;

static TwEnumVal integrationEV[] = {
    
    { Cloth::F_EULER,   "FB Forward Euler"},
    { Cloth::RK2_EULER, "FB Runge-Kutta 2" },
    { Cloth::RK4_EULER, "FB Runge-Kutta 4" },
    { Cloth::FB_EULER,  "FB Symplectic Euler" },
    { Cloth::MFB_EULER, "FB IMplicit-EXplicit" },
    { Cloth::B_EULER,   "FB Backward Euler" },
    { Cloth::VERLET,    "PB Verlet"}
};

static TwEnumVal presetEV[] = {
    
    { Cloth::One, "Preset One" },
    { Cloth::Two, "Preset Two" }
};

// TODO: Fix camera, only works when alligned to a single axis, also position resets after click 17/03/16
static TrackBall tBall(WIN_DIMENSION_WIDTH, WIN_DIMENSION_HEIGHT, 3.5, GL_TRUE, GL_FALSE);

/// CALLBACK FUNCTIONS
static GLvoid TW_CALL ReInit(void*) {
    
    clothObject = new Cloth(UNIT_WIDTH, UNIT_HEIGHT, NxN, NxN, preset);
    Nc = pow(NxN, 2);
}

static GLvoid TW_CALL Pause(void*) {
    
    pausePlay = !pausePlay;
}

static GLvoid keyCallback(GLFWwindow* window, GLint key, GLint scancode, GLint action, GLint mods) {
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    
    TwEventKeyGLFW(key, action);
    TwEventCharGLFW(key, action);
}

static GLvoid mouseButtonCallback(GLFWwindow* window, GLint button, GLint action, GLint mods){
    
    if (!(TwEventMouseButtonGLFW(button, action))) {
        tBall.mouseButtonCallback(window, button, action, mods);
    }
}

static GLvoid cursorCallback(GLFWwindow* window, GLdouble x, GLdouble y) {
    
    if (!(TwEventMousePosGLFW(x * (pixWidth / corWidth), y * (pixHeight / corHeight)))) { // ATB is not designed for Retina/HiDPI Displays, map screen space coordinates to GL pixel coordinates
        tBall.cursorCallback(window, x, y);
    }
}

static GLvoid windowSizeCallback(GLFWwindow* window, GLint width, GLint height) {
    TwWindowSize(width, height);
}

GLvoid frameBuffer(void) {
    
    glfwGetFramebufferSize(gWindow, &pixWidth, &pixHeight);
    glfwGetWindowSize(gWindow, &corWidth, &corHeight);
}

GLfloat randFunc(GLfloat v) {
    return (-v) + ((GLfloat)rand() / RAND_MAX) * ((v) - (-v));
}

GLvoid glInfo(void) {
    
    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "OpenGL Version Supported: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    
    // Print the GLFW version
    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    cout << "GLFW Version: " << major << "." << minor << "." << rev << endl;
    cout << endl;
}

GLvoid initATB(void) {
    
    // check for
    if (!((corWidth == pixWidth) && (corHeight == pixHeight))) {
    
        TwDefine(" GLOBAL fontscaling = 2.00 ");
        TwDefine(" GLOBAL fontsize = '2' ");
    }
    
    TwInit(TW_OPENGL_CORE, NULL);
    
    TwDefine(" GLOBAL iconmargin = '36 32' ");
    TwDefine(" GLOBAL contained = 'true' ");
    
    // Cloth Parameters Bar Initialisations
    tMain = TwNewBar("Cloth Parameters");
    TwDefine(" 'Cloth Parameters' size = '700 800' ");
    TwDefine(" 'Cloth Parameters' refresh = '0.0001f' ");
    TwDefine(" 'Cloth Parameters' alpha = '192' ");
    
    TwType integrationType = TwDefineEnum("Integration Mode", integrationEV, 7);
    TwType presetType = TwDefineEnum("Preset", presetEV, 2);

    // 'Cloth Parameters' Bar Variable Tweak Initialisations
    
    TwAddVarRW(tMain, "Preset", presetType, &preset, "group = 'Preset & Dimensions'");
    TwAddVarRW(tMain, "NxN", TW_TYPE_INT16, &NxN, "group = 'Preset & Dimensions' min = 1 max = 100 step = 1");
    TwAddVarRO(tMain, "Node Count", TW_TYPE_INT16, &Nc, "group = 'Preset & Dimensions'");
    TwAddButton(tMain, "Re-instantiate", ReInit, NULL, "group = 'Preset & Dimensions'");
    
    
    TwAddVarRW(tMain, "Timestep", TW_TYPE_FLOAT, &timestep, "group = 'Integration' min = 0.00001 max = 0.7000 step = 0.00085 help = 'Simulation step size'");
    TwAddVarRW(tMain, "Scheme", integrationType, &integration, "group = 'Integration'");
    TwAddButton(tMain, "Pause/Play", Pause, NULL, "group = 'Integration'");
    
    TwAddVarRW(tMain, "Global Damping", TW_TYPE_FLOAT, &globalDamping,  "group = 'Constraints' min = -1.000 max = 0.000, step = 0.005 help = 'Particle velocity damping'");
    
    TwAddVarRW(tMain, "Stretch Stiffness", TW_TYPE_FLOAT, &stretchStiffness, "group = 'Stretch' min = 0.000 max = 300.00, step = 0.50 label = 'Stiffness'");
    TwAddVarRW(tMain, "Stretch Damping", TW_TYPE_FLOAT, &stretchDamping, "group = 'Stretch' min = -1.000 max = 0.000, step = 0.001 label = 'Damping'");
    TwDefine(" 'Cloth Parameters'/Stretch group = 'Constraints' ");
    
    TwAddVarRW(tMain, "Shear Stiffness", TW_TYPE_FLOAT, &shearStiffness, "group = 'Shear' min = 0.000 max = 200.00, step = 0.25 label = 'Stiffness'");
    TwAddVarRW(tMain, "Shear Dampings", TW_TYPE_FLOAT, &shearDamping, "group = 'Shear' min = -1.000 max = 0.000, step = 0.001 label = 'Damping'");
    TwDefine(" 'Cloth Parameters'/Shear group = 'Constraints' ");

    TwAddVarRW(tMain, "Bend Stiffness", TW_TYPE_FLOAT, &bendStiffness, "group = 'Bend' min = 0.000 max = 100.00, step = 0.05 label = 'Stiffness'");
    TwAddVarRW(tMain, "Bend Damping", TW_TYPE_FLOAT, &bendDamping, "group = 'Bend' min = -1.000 max = 0.000, step = 0.001 label = 'Damping'");
    TwDefine(" 'Cloth Parameters'/Bend group = 'Constraints' ");
    
    TwAddVarRW(tMain, "Draw Mesh", TW_TYPE_BOOLCPP, &drawW, "group = 'Meshing & Highlighting'");
    
    TwAddVarRW(tMain, "Show Normals", TW_TYPE_BOOLCPP, &normals, "group = 'Normal Mapping'");
    TwAddVarRW(tMain, "Length", TW_TYPE_FLOAT, &nLength, "group = 'Normal Mapping'");
    TwAddVarRW(tMain, "Normal Highlight Vibrance", TW_TYPE_FLOAT, &nVibrance, "group = 'Normal Mapping' label = 'Vibrance'");

    TwDefine(" 'Cloth Parameters'/'Normal Mapping' group = 'Meshing & Highlighting'");

    TwAddVarRW(tMain, "Show Forces", TW_TYPE_BOOLCPP, &forces, "group = 'Force Colouring'");
    TwAddVarRW(tMain, "Force Highlight Vibrance", TW_TYPE_FLOAT, &fVibrance, "group = 'Force Colouring' label = 'Vibrance'");
    
    TwDefine(" 'Cloth Parameters'/'Force Colouring' group = 'Meshing & Highlighting'");
    
    TwAddVarRW(tMain, "Show Velocities", TW_TYPE_BOOLCPP, &velocities, "group = 'Velocity Colouring'");
    TwAddVarRW(tMain, "Velocity Highlight Vibrance", TW_TYPE_FLOAT, &vVibrance, "group = 'Force Colouring' label = 'Vibrance'");
    
    TwDefine(" 'Cloth Parameters'/'Velocity Colouring' group = 'Meshing & Highlighting'");
    
    TwAddSeparator(tMain, NULL, NULL);
    TwAddVarRW(tMain, "Enable Self Collisions [EXP]", TW_TYPE_BOOLCPP, &selfCol, NULL);
    TwAddVarRW(tMain, "Enable Dynamic Inverse", TW_TYPE_BOOLCPP, &inverseDy, "help = 'Toggle constraint inverse dynamics, X. Provot [1995]'");
    
    TwDefine(" 'Cloth Parameters' valueswidth = '300' ");
    
    // Subsidiary Parameters Bar Initialisations
    
    tSub = TwNewBar("Subsidiary Parameters");
    TwDefine(" 'Subsidiary Parameters' size = '600 620' ");
    TwDefine(" 'Subsidiary Parameters' position = '60 850' ");
    TwDefine(" 'Subsidiary Parameters' alpha = '192' ");
    
    // 'Subsidiary Parameters' Bar Variable Tweak Initialisations
    
    TwAddVarRW(tSub, "Direction", TW_TYPE_DIR3F, &windDirection, "group = 'Wind'");
    
    TwAddVarRW(tSub, "Intensity", TW_TYPE_FLOAT, &wIntensity, "group = 'Properties' min = '0.01' max = '50' step = 0.05");
    TwAddVarRW(tSub, "PP Turbulance", TW_TYPE_FLOAT, &ppTurbulance, "group = 'Properties' min = '0.00' max = '5' step = 0.01");
    TwAddVarRW(tSub, "Variance X", TW_TYPE_FLOAT, &variance.x, "group = 'Properties' min = '0.00' max = '5' step = 0.01");
    TwAddVarRW(tSub, "Variance Y", TW_TYPE_FLOAT, &variance.y, "group = 'Properties' min = '0.00' max = '5' step = 0.01");
    TwAddVarRW(tSub, "Variance Z", TW_TYPE_FLOAT, &variance.z, "group = 'Properties' min = '0.00' max = '5' step = 0.01");
    
    TwDefine(" 'Subsidiary Parameters'/Properties group = 'Wind' ");

    TwAddSeparator(tSub, NULL, NULL);
    
    TwAddVarRW(tSub, "Enable Wind", TW_TYPE_BOOLCPP, &wind, NULL);
    TwAddVarRW(tSub, "Enable Planar Collisions", TW_TYPE_BOOLCPP, &planarCol, NULL);
    TwAddVarRW(tSub, "Enable Sphere Collisions", TW_TYPE_BOOLCPP, &sphereCol, NULL);
    
    // Scene Manager Bar Initialisations
    
    tScene = TwNewBar("Shading Properties");
    TwDefine(" 'Shading Properties' size = '600 700' ");
    TwDefine(" 'Shading Properties' position = '2260 20' ");
    TwDefine(" 'Shading Properties' alpha = '192' ");

    
    // 'Scene Manager' Bar Variable Tweak Initialisations
    /*
    TwAddVarRW(tScene, "Draw Cloth", TW_TYPE_BOOLCPP, &drawCloth, NULL);
    TwAddVarRW(tScene, "Draw Wire", TW_TYPE_BOOLCPP, &drawW, NULL);
    TwAddVarRW(tScene, "Draw Plane", TW_TYPE_BOOLCPP, &drawPlane, NULL);
    */
    
    //TwAddVarRW(tScene, "Draw Sphere", TW_TYPE_BOOLCPP, &drawSphere, NULL);
    
    TwAddVarRW(tScene, "L Ambient Colour", TW_TYPE_COLOR3F, &light.ambientColour, "group = 'Lighting Properties'");
    TwAddVarRW(tScene, "L Diffuse Colour", TW_TYPE_COLOR3F, &light.diffuseColour, "group = 'Lighting Properties'");
    TwAddVarRW(tScene, "L Specular Colour", TW_TYPE_COLOR3F, &light.specularColour, "group = 'Lighting Properties'");
    TwAddVarRW(tScene, "L Light Position", TW_TYPE_DIR3F, &light.position, "group = 'Lighting Properties'");
    
    TwAddVarRW(tScene, "M Ambient Colour", TW_TYPE_COLOR3F, &material.ambientColour, "group = 'Material Properties'");
    TwAddVarRW(tScene, "M Diffuse Colour", TW_TYPE_COLOR3F, &material.diffuseColour, "group = 'Material Properties'");
    TwAddVarRW(tScene, "M Specular Colour", TW_TYPE_COLOR3F, &material.specularColour, "group = 'Material Properties'");
    TwAddVarRW(tScene, "M Specular Shininess ", TW_TYPE_FLOAT, &material.specularShininess, "group = 'Material Properties'");
    
    TwAddVarRW(tScene, "Background Colour", TW_TYPE_COLOR3F, &backgroundC, NULL);

}

GLvoid initShadingVars(void) {
    
    material.ambientColour = vec3(0.0f, 0.3215686275f, 0.7137254902f);
    material.diffuseColour = vec3(0.2039215686f, 0.5843137255f, 0.8235294118f);
    material.specularColour = vec3(1, 1, 1);
    material.specularShininess = 20.0f;
    
    light.ambientColour = vec3(0.1725490196f, 0.3764705882f, 0.462745098f);
    light.diffuseColour = vec3(1, 1, 1);
    light.specularColour = vec3(1, 1, 1);
    
    light.position = vec3(2.96f, 12.09f, 9.56f);
}

GLvoid setClothParameters(Cloth* cloth) {
    
    cloth->setStiffness(stretchStiffness, shearStiffness, bendStiffness);
    cloth->setDamping(stretchDamping, shearDamping, shearDamping, globalDamping);
    
    if (sphereCol)
        cloth->sphereCollision(sphereObject->getSpherePosition(), sphereObject->getSphereRadius());
    
    if (planarCol)
        cloth->planarCollision(0);
    
    if (selfCol)
        cloth->selfCollision();
    
    cloth->setWire(drawW);
    
    if (clothReInit) {
        
        clothObject = new Cloth(UNIT_WIDTH, UNIT_HEIGHT, NxN, NxN, preset);
        
        Nc = pow(NxN, 2);
    }
    
    if (wind) {
        
        vec3 forceVec = windDirection * wIntensity;
        forceVec + vec3(randFunc(variance.x), randFunc(variance.y), randFunc(variance.z));
        
        cloth->setWind(forceVec, vec3(ppTurbulance));
    }
}

// TODO: Required Feature, AntTweakBar or imGUI implementation

GLint main(GLint argc, const GLchar* argv[]) {
    
    if (!glfwInit()) {
        return 1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x Super Sampling Anti-Aliasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    
    gWindow = glfwCreateWindow(WIN_DIMENSION_WIDTH, WIN_DIMENSION_HEIGHT, APP_NAME, NULL, NULL);
    
    if (gWindow == NULL) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        
        glfwTerminate();
        
        return 1;
    }
    
    glfwMakeContextCurrent(gWindow);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        glfwTerminate();
        exit(-1);
    }
    
    frameBuffer();
    
    glInfo();
    
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glDepthFunc(GL_LESS);
    
    // Set Callback Functions
    glfwSetCursorPosCallback(gWindow, cursorCallback);
    glfwSetMouseButtonCallback(gWindow, mouseButtonCallback);
    glfwSetKeyCallback(gWindow, keyCallback);
        glfwSetWindowSizeCallback(gWindow, windowSizeCallback);
    
    /// CLOTH PROGRAM INITIALISATIONS
    
    mainShader = new Shader();
    subShader = new Shader();

    mainShader->ProgramID = mainShader->LoadShaders(LIGHTING_VERTEX_SHADER, LIGHTING_FRAGMENT_SHADER);
    mainShader->initShaders();
    
    subShader->ProgramID = subShader->LoadShaders(NORMAL_VERTEX_SHADER, GEOMETRY_SHADER_PATH, NORMAL_FRAGMENT_SHADER);
    subShader->initShaders();
    
    plane = new ObjectMesh();
    plane->initPlane(PLANE_DIMENSIONS);
    
    sphereObject = new ObjectMesh();
    sphereObject->initSphere(SPHERE_DIMENSIONS, vec3(0, (GLfloat)UNIT_HEIGHT / 2.5f, 0));
    
    clothObject = new Cloth(UNIT_WIDTH, UNIT_HEIGHT, NO_PARTICLES_X, NO_PARICLES_Y, preset);
    
    glfwSetKeyCallback(gWindow, keyCallback);
    
    /// Experimental MVP Code
    
    mat4 Model = mat4(1.0); // Identity/Unit Matrix
    mat4 View = lookAt(vec3(0.0f, 0.0f, 3.0f), vec3(0), vec3(0.0f, 1.0f, 0.0f));
    
    initShadingVars();
    
    initATB();
    
    GLint count = 0;
    
    
    while (!glfwWindowShouldClose(gWindow)) {
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(backgroundC.r, backgroundC.g, backgroundC.b, 1.0);

        frameBuffer();
        
        TwWindowSize(pixWidth, pixHeight);
    
        glUseProgram(mainShader->ProgramID);
        // glUseProgram(subShader->ProgramID);
        
        mat4 Projection = perspective(FoV, ((GLfloat)pixWidth) / ((GLfloat)pixHeight), 0.1f, 100.0f);
        mat4 RotatedModel = Model * tBall.createModelRotationMatrix(View);
        
        glUniformMatrix4fv(glGetUniformLocation(mainShader->GetProgramID(), "u_model"), 1, GL_FALSE, value_ptr(RotatedModel));
        glUniformMatrix4fv(glGetUniformLocation(mainShader->GetProgramID(), "u_view"), 1, GL_FALSE, value_ptr(View));
        glUniformMatrix4fv(glGetUniformLocation(mainShader->GetProgramID(), "u_projection"), 1, GL_FALSE, value_ptr(Projection));

        glUniform3fv(glGetUniformLocation(mainShader->GetProgramID(), "u_lightDir"), 1, value_ptr(LightDirection));
        
        glUniform3fv(glGetUniformLocation(mainShader->GetProgramID(), "light.ambientColour"), 1, value_ptr(light.ambientColour));
        glUniform3fv(glGetUniformLocation(mainShader->GetProgramID(), "light.diffuseColour"), 1, value_ptr(light.diffuseColour));
        glUniform3fv(glGetUniformLocation(mainShader->GetProgramID(), "light.specularColour"), 1, value_ptr(light.specularColour));

        glUniform3fv(glGetUniformLocation(mainShader->GetProgramID(), "light.position"), 1, value_ptr(light.position));
        glUniform3fv(glGetUniformLocation(mainShader->GetProgramID(), "u_lightPos"), 1, value_ptr(light.position));

        glUniform3fv(glGetUniformLocation(mainShader->GetProgramID(), "material.ambientColour"), 1, value_ptr(material.ambientColour));
        glUniform3fv(glGetUniformLocation(mainShader->GetProgramID(), "material.diffuseColour"), 1, value_ptr(material.diffuseColour));
        glUniform3fv(glGetUniformLocation(mainShader->GetProgramID(), "material.specularColour"), 1, value_ptr(material.specularColour));
        
        glUniform1f(glGetUniformLocation(mainShader->GetProgramID(), "material.specularShininess"), material.specularShininess);
        
        setClothParameters(clothObject);
        
        if (!pausePlay)
            clothObject->stepSimulation(timestep, integration, inverseDy);
        
        if (drawCloth) {
            
            clothObject->render(mainShader);
            //clothObject->render(subShader);
        }

        if (drawPlane)
            plane->renderPlane(mainShader);
        
        if (drawSphere)
            sphereObject->renderSphere(mainShader);
        
    
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        TwDraw();
        
        glfwSwapBuffers(gWindow);
        glfwPollEvents();
        
        count++;
    }
    
    TwTerminate();
    glfwTerminate();
    
    return(0);
}