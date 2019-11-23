#version 430 core

uniform mat4 modelview;
uniform mat4 projection;

layout (location = 0) in vec3 position;

out vec3 N;
out vec3 P;
out vec3 L;

uniform vec3 lightPosition;

void main (void)
{	
    mat3 normalMatrix = mat3(transpose(inverse(modelview)));
    N = normalize(normalMatrix*vec3(0.0, 1.0, 0.0));
    vec4 L4 = modelview * vec4(lightPosition, 1.0);
    vec4 P4 = modelview * vec4( position.x, position.y - 4, position.z, 1.0);
    L = L4.xyz/L4.w;
    P = P4.xyz/P4.w;

    gl_Position = projection *  P4;
}
