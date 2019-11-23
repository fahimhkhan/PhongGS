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
	glm::vec3 Fs;
	glm::vec3 Fd;
	float m;
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

	void CreateSpring(int lmin);

	void renderPlane();
	void renderSpring(glm::vec3 position1, glm::vec3 position2);
	void renderMass(glm::vec3 init_position);
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

	Mass *P;
};

#endif /* SCENESHADER_H_ */
