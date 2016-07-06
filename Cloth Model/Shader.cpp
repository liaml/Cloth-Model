#include "Shader.hpp"

using namespace std;
using namespace glm;

Shader::Shader(void)
{
}

Shader::~Shader(void) {
    glDeleteProgram(ProgramID);
}

GLuint Shader::LoadShaders(const char* vFPath, const char* gFPath, const char* fFPath) {
    
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read the Vertex Shader code from the file
    string VertexShaderCode;
    ifstream VertexShaderStream(vFPath, ios::in);
    if (VertexShaderStream.is_open()){
        string Line = "";
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else{
        printf("Cannot open the path : %s\n", vFPath);
        getchar();
        return 0;
    }
    
    // Read the Fragment Shader code from the file
    string FragmentShaderCode;
    ifstream FragmentShaderStream(fFPath, std::ios::in);
    if (FragmentShaderStream.is_open()){
        std::string Line = "";
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    else{
        printf("Cannot open the path : %s\n", fFPath);
        getchar();
        return 0;
    }
    
    // Read the Geometry Shader code from the file
    string GeometryShaderCode;
    ifstream GeometryShaderStream(gFPath, std::ios::in);
    if (GeometryShaderStream.is_open()){
        std::string Line = "";
        while (getline(GeometryShaderStream, Line))
            GeometryShaderCode += "\n" + Line;
        GeometryShaderStream.close();
    }
    else{
        printf("Cannot open the path : %s\n", gFPath);
        getchar();
        return 0;
    }
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vFPath);
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);
    
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0){
        vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }
    
    // Compile Vertex Shader
    printf("Compiling shader : %s\n", gFPath);
    char const* GeometrySourcePointer = GeometryShaderCode.c_str();
    glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer, NULL);
    glCompileShader(GeometryShaderID);
    
    // Check Vertex Shader
    glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0){
        vector<char> GeometryShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(GeometryShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
        printf("%s\n", &GeometryShaderErrorMessage[0]);
    }
    
    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fFPath);
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0){
        vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }
    
    // Link the program
    printf("Linking program\n\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, GeometryShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    
    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0){
        vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(GeometryShaderID);
    glDeleteShader(FragmentShaderID);
    
    return ProgramID;
}

GLuint Shader::LoadShaders(const char* vFPath, const char* fFPath) {
    
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read the Vertex Shader code from the file
    string VertexShaderCode;
    ifstream VertexShaderStream(vFPath, ios::in);
    if (VertexShaderStream.is_open()){
        string Line = "";
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else{
        printf("Cannot open the path : %s\n", vFPath);
        getchar();
        return 0;
    }
    
    // Read the Fragment Shader code from the file
    string FragmentShaderCode;
    ifstream FragmentShaderStream(fFPath, std::ios::in);
    if (FragmentShaderStream.is_open()){
        std::string Line = "";
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    else{
        printf("Cannot open the path : %s\n", fFPath);
        getchar();
        return 0;
    }
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vFPath);
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);
    
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0){
        vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }
    
    
    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fFPath);
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0){
        vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }
    
    // Link the program
    printf("Linking program\n\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    
    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0){
        vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    
    return ProgramID;
}

GLuint Shader::GetProgramID() {
    return ProgramID;
}

GLvoid Shader::UseProgram(GLuint ProgramID) {
    glUseProgram(ProgramID);
}

GLvoid Shader::initShaders(void) {
    
    lightPos0 = vec4(-1.0, 1.0, 0.5, 0.0);
    
    vec4 lightAmbient0 = vec4(0.2, 0.2, 0.2, 1.0);
    vec4 lightDiffuse0 = vec4(0.8, 0.8, 0.8, 1.0);
    
    lightPos1 = vec4(1.0, 0.0, -0.2, 0.0);
    
    vec4 lightAmbient1 = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 lightDiffuse1 = vec4(0.5, 0.5, 0.3, 0.0);
    
    vec4 ambient[2] = {lightAmbient0, lightAmbient1};
    vec4 diffuse[2] = {lightDiffuse0, lightDiffuse1};
    
    glUseProgram(ProgramID);
    
    glUniform4fv(glGetUniformLocation(ProgramID, "lightAmbient"), 2, value_ptr(ambient[0]));
    glUniform4fv(glGetUniformLocation(ProgramID, "lightDiffuse"), 2, value_ptr(diffuse[0]));
    
    vec4 lightModelAmbient = vec4(0.2, 0.2, 0.2, 1.0);
    glUniform4fv(glGetUniformLocation(ProgramID, "lightModelAmbient"), 1, value_ptr(lightModelAmbient));
    
    vec4 eyeSpaceLight[2] = {lightPos0, lightPos1};
    glUniform4fv(glGetUniformLocation(ProgramID, "lightPosition"), 2, value_ptr(eyeSpaceLight[0]));
}