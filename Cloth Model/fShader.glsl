#version 330 core

#define FLAT_LBLUE 0.1607843137, 0.5019607843, 0.7254901961 // FUIC BELIZE HOLE
#define FLAT_LGREEN 0.1803921569, 0.8, 0.4431372549 // FUIC EMERALD
#define FLAT_DBLUE 0.1725490196, 0.2274509804, 0.2862745098 // FCUI K11
#define FLAT_BLACK 0.1137254902, 0.1137254902, 0.1137254902 // CL DARKEN
#define FLAT_WHITE 0.7843137255, 0.8117647059, 0.8392156863
#define FIGURE_CAPTURE 0,0,0

layout(location = 0) out vec4 fragColour;

in vec3 vertexColour;

void main()
{
    fragColour = vec4(vertexColour, 1.0);
}