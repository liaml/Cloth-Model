#include "TrackBall.hpp"

using namespace glm;

/**
 * Constructor.
 * @param rSpeed the speed of rotation
 */
TrackBall::TrackBall(GLuint winWidth, GLuint winHeight, GLfloat rollSpeed, GLboolean xAxis, GLboolean yAxis) {
    
    _wWidth   = winWidth;
    _wHeight  = winHeight;
    
    _mEvent   = 0;
    _rSpeed   = rollSpeed;
    _angle    = 0.0f;
    _camAxis  = vec3(0.0f, 1.0f, 0.0f);
    
    _xAxis    = xAxis;
    _yAxis    = yAxis;
}

/**
 * Convert the mouse cursor coordinate on the window (i.e. from (0,0) to (windowWidth, windowHeight))
 * into normalized screen coordinate (i.e. (-1, -1) to (1, 1)
 */
vec3 TrackBall::toScreenCoord(GLdouble x, GLdouble y) {
    
    vec3 coord(0.0f);
    
    if(_xAxis)
        coord.x =  (2 * x - _wWidth ) / _wWidth;
    
    if(_yAxis)
        coord.y = -(2 * y - _wHeight) / _wHeight;
    
    /* Clamp it to border of the windows, comment these codes to allow rotation when cursor is not over window */
    coord.x = clamp(coord.x, -1.0f, 1.0f);
    coord.y = clamp(coord.y, -1.0f, 1.0f);
    
    GLfloat length2 = coord.x * coord.x + coord.y * coord.y;
    
    if( length2 <= 1.0 )
        coord.z = glm::sqrt(1.0 - length2);
    else
        coord = normalize(coord);
    
    return coord;
}

/**
 * Check whether we should start the mouse event
 * Event 0: when no tracking occured
 * Event 1: at the start of tracking, recording the first cursor pos
 * Event 2: tracking of subsequent cursor movement
 */

GLvoid TrackBall::mouseButtonCallback(GLFWwindow* window, GLint button, GLint action, GLint mods) {
    
    _mEvent = (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT);
}

GLvoid TrackBall::cursorCallback(GLFWwindow* window, GLdouble x, GLdouble y) {
    
    if(_mEvent == 0) {
        
        return;
    }
    else if(_mEvent == 1) {
        
        /* Start of trackball, remember the first position */
        _lastPos = toScreenCoord(x, y);
        _mEvent = 2;
        
        return;
    }
    
    /* Tracking the subsequent */
    _position = toScreenCoord(x, y);
    
    /* Calculate the angle in radians, and clamp it between 0 and 90 degrees */
    _angle = acos(min(1.0f, glm::dot(_lastPos, _position)));
    
    /* Cross product to get the rotation axis, but it's still in camera coordinate */
    _camAxis  = cross(_lastPos, _position);
}

/**
 * Create rotation matrix within the camera coordinate,
 * multiply this matrix with view matrix to rotate the camera
 */
inline mat4 TrackBall::createViewRotationMatrix(void) {
    return rotate(degrees(_angle) * _rSpeed, _camAxis);
}

/**
 * Create rotation matrix within the world coordinate,
 * multiply this matrix with model matrix to rotate the object
 */
mat4 TrackBall::createModelRotationMatrix(mat4& viewMatrix){
    return rotate(degrees(_angle) * _rSpeed, (inverse(mat3(viewMatrix)) * _camAxis));
}