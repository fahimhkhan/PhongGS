/*
 * SceneShader.h
 *
 */

#ifndef SCENESHADER_H_
#define SCENESHADER_H_

#include "Shader.h"
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

class Boid{
public:
	glm::vec3 x;	//Position
	glm::vec3 v;	//Velocity
	glm::vec3 F;	//Force	
	glm::mat4 M;	//Orientation
	float m;
};

class Force{
public:
	int id1;
	int id2;
	float l;
	float ks;
	float kd;
};

class SceneShader : public Shader
{
public:

	SceneShader();
	~SceneShader();

	void startup ();
	void shutdown ();
	void render();
	void simulation(float t);
	void play();

	void renderPlane();
	void renderObs(glm::vec3 init_position);
	void renderBoid(glm::vec3 init_position, glm::mat4 orientation, glm::vec3 color);
	void renderLight(glm::vec3 init_position, glm::vec3 color, float rp);

	void setAspectRatio( float ratio );
	void setZTranslation(float z);
	void setRotationX( float x );
	void setRotationY( float y );
	void setRotationZ(float z);

	void setGXY(float Gx, float Gy);

	void updateLightPositionX( float x );
	void updateLightPositionY( float y );
	void updateLightPositionZ( float z );

	void updateDisplacement(float y);

private:

	/*methods*/
	void createVertexBuffer();


	/*variables*/
	GLuint _programLight;
	GLuint _programPlane;
	GLuint _programBoid;
	GLuint _programForce;

	GLuint _planeVertexArray;
	GLuint _planeVertexBuffer;

	GLuint _ObsVertexArray;
	GLuint _ObsVertexBuffer;

	GLuint _boidVertexArray;
	GLuint _boidVertexBuffer;


    GLint _mvUniform, _projUniform;

	/* Matrices */
	glm::mat4 _modelview;
	glm::mat4 _projection;

	float _zTranslation;
	float _xRot;
	float _yRot;
	float _zRot;
	float _aspectRatio;
	bool g_play;

	float parameter[10];

	/*vertices*/
	std::vector<float> _vertices;

	std::vector<unsigned int> _triangleIndices;

	glm::vec3 lightPosition;
	float Displacement;
	int nBoid;
	int nForce;
	int size;

	Boid *P;
	Force *Sp;
};

#endif /* SCENESHADER_H_ */
