#version 430 core

uniform mat4 modelview;
uniform mat4 projection;
uniform mat4 rotation;
uniform vec3 lightPosition;
uniform float scale;
uniform vec3 translate;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;

//attributes in camera coordinates
out vec3 N; //velocity
out vec3 V; //view vector
out vec3 L; //light
out vec3 C;
out vec3 vPosition;
out vec3 vNormal;

void main(void)
{
    vec4 lightCameraSpace = modelview * vec4(lightPosition, 1.0);
    lightCameraSpace /= lightCameraSpace.w;

    mat3 normalMatrix = mat3(transpose(inverse(modelview)));
    //N = normalize(normalMatrix * normal);
	N = normalize(normal);
	C = normalize(vertex);
	
	vec4 vertex1 = vec4(vertex, 1.0);
	vertex1 = vertex1*rotation;      
	vec3 positionModelSpace = (vertex1.xyz)*scale + translate;
	vec4 p4 = vec4(positionModelSpace, 1.0);
	vec4 positionCameraSpace = modelview * p4;
    //vec4 positionCameraSpace = modelview * vec4(positionModelSpace, 1.0);

    vec3 P = positionCameraSpace.xyz/positionCameraSpace.w;
   
    L = normalize(lightCameraSpace.xyz - P);
    V = normalize(-P);
 
    gl_Position = projection * positionCameraSpace;   
	vPosition = vertex.xyz;
    vNormal = vertex;

}

