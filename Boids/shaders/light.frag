#version 430 core

uniform vec3 inColor;
out vec4 color;

void main(void)
{    
    color = vec4(inColor, 1.0);
}
