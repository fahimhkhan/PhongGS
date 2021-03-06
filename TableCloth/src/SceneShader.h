/*
 * SceneShader.h
 *
 */

#ifndef SCENESHADER_H_
#define SCENESHADER_H_

#include "Shader.h"
#include <vector>

#define GLM_FORCE_RADIANS


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Mass{
public:
	glm::vec3 x;
	glm::vec3 v;
	glm::vec3 F;
	float m;
};

class Spring{
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

	void CreateSprings(int lmin);

	void renderPlane();
	void renderSpring(glm::vec3 position1, glm::vec3 position2);
	void renderTriangle(glm::vec3 position1, glm::vec3 position2, glm::vec3 position3);
	void renderLight();

	void setAspectRatio( float ratio );
	void setZTranslation(float z);
	void setRotationX( float x );
	void setRotationY( float y );
	void setRotationZ(float z);

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
	GLuint _programMass;
	GLuint _programSpring;

	GLuint _planeVertexArray;
	GLuint _planeVertexBuffer;

	GLuint _springVertexArray;
	GLuint _springVertexBuffer;

	GLuint _massVertexArray;
	GLuint _massVertexBuffer;


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


	/*vertices*/
	std::vector<float> _vertices;

	std::vector<unsigned int> _triangleIndices;

	glm::vec3 lightPosition;
	float Displacement;
	int nMass;
	int nSpring;
	int size;

	Mass *P;
	Spring *Sp;
};

#endif /* SCENESHADER_H_ */
