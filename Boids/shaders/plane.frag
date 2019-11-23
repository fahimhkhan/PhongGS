#version 430 core

out vec4 color;

in vec3 N;
in vec3 P;
in vec3 L;

uniform vec3 inColor;

void main (void)
{
    vec3 diffuse_albedo = inColor;
    float kd = 0.8;
    vec3 diffuse = kd*diffuse_albedo*max( 0.0, dot( N, normalize(L - P)));
    color = vec4( diffuse, 1.0);
}
