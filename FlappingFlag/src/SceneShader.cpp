/*
 * SceneShader.cpp
 * Created by: Fahim Hasan Khan
 * Date :23-Mar-2017
 *
 */

#include "SceneShader.h"
static float PI = 3.14159265359;
float t = 0.0;
float dt = 1.0;
float ks = 50.0;	//Spring stiffness
float kd = 0.5;		//Damping coefficient
float l = 0.25;		//Rest length of spring
glm::vec3 G(0.0, -9.8, 0.0); //Gravity

std::vector<glm::vec3> cverts;
std::vector<glm::vec3> tris;

SceneShader::SceneShader(): Shader()
{
	_programPlane = 0;
	_programSpring = 0;
	_programMass = 0;
	_planeVertexArray = -1;
	_planeVertexArray = -1;
	_mvUniform = -1;
	_projUniform = -1;
	_zTranslation = 1.0;
	_aspectRatio = 1.0;
	_xRot = 0.0;
	_yRot = 0.0;
	_zRot = 0.0;
	//lightPosition = glm::vec3(0.5, 0.5, 0.5);
	lightPosition = glm::vec3(10.0, 10.0, 10.0);
	g_play = true;
	nMass = 24*24;

	P = (Mass*)malloc((nMass+1)*sizeof(Mass));

}

void SceneShader::simulation(float t) {
	int size = 25;
	auto id = [size](int i, int j) { return (size)*i + j; };



	for (int i = 0; i < size; i++) 
		for (int j = 0; j < size; j++){
			int a = id(i, j);
			cverts[a].x = float((i / 5.0f) - 4.5f);
			cverts[a].y = float((j / 5.0f) - 4.5f);
			cverts[a].z = float(sin((((i+t / 5.0f)*20.0f) / 360.0f)*PI*2.0f));

	}

		for (int i = 0; i<(2 * nMass); i++)
			renderTriangle(cverts[(tris[i].x)], cverts[(tris[i].y)], cverts[(tris[i].z)]);
}

void SceneShader::CreateSpring(int lmin){
	return;
}


void SceneShader::createVertexBuffer()
{
	//create plane geometry
	static const GLfloat quadData[] =
	{
		-4.0f, 0.0f, -4.0f,
		-4.0f, 0.0f, 4.0f,
		4.0f, 0.0f, -4.0f,
		4.0f, 0.0f, 4.0f,
	};


	
	//passing model attributes to the GPU
	//plane
	glGenVertexArrays(1, &_planeVertexArray);
	glBindVertexArray(_planeVertexArray);

	glGenBuffers(1, &_planeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _planeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof (quadData), quadData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);


	//Cloth
	int size = 25;
	float scale = 0.05f;
	
	for (int r = 0; r < size; ++r) {
		for (int c = 0; c < size; ++c) {
			// push back Vertex( position, rgb )
			float x = (c - size * 0.5) * scale;
			float y = (r - size * 0.5) * scale;

			cverts.push_back(glm::vec3(x, y, 0.0));
			//printf("\n%f,%f", x, y);
		}
	}
	// helper lambda function to get array id from row, column
	auto id = [size](int r, int c) { return (size)*r + c; };

	for (int row = 0; row < size - 1; ++row) {
		for (int col = 0; col < size - 1; ++col) {
			int a = id(row, col);
			int b = id(row, col + 1);
			int c = id(row + 1, col);
			int d = id(row + 1, col + 1);

			tris.push_back(glm::vec3(a, b, c));
			tris.push_back(glm::vec3(c, b, d));
		}
	}

}


void SceneShader::startup()
{
	

	_programPlane = compile_shaders("./shaders/plane.vert", "./shaders/plane.frag");
	_programMass = compile_shaders("./shaders/mass.vert", "./shaders/mass.frag");
	_programLight = compile_shaders("./shaders/light.vert", "./shaders/light.frag");
	_programSpring = compile_shaders("./shaders/spring.vert", "./shaders/spring.frag");

	createVertexBuffer();

}

void SceneShader::renderPlane()
{
	glBindVertexArray(_planeVertexArray);

	glUseProgram(_programPlane);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, 15.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);

	_modelview = glm::lookAt( eye, center, up);

	glm::mat4 identity(1.0f);
	_projection = glm::perspective( 45.0f, _aspectRatio, 0.01f, 100.0f);

	glm::mat4 rotationX = glm::rotate(identity, _xRot  * PI / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	_modelview *= rotationX;

	glm::mat4 rotationY = glm::rotate(identity, _yRot  * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	_modelview *= rotationY;

	glm::mat4 rotationZ = glm::rotate(identity, _zRot  * PI / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));

	_modelview *= rotationZ;
	
	//Uniform variables
	glUniformMatrix4fv(glGetUniformLocation(_programPlane, "modelview"), 1, GL_FALSE, glm::value_ptr(_modelview));
	glUniformMatrix4fv(glGetUniformLocation(_programPlane, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));

	glUniform3fv(glGetUniformLocation(_programPlane, "lightPosition"), 1, glm::value_ptr(lightPosition) );

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
}

void SceneShader::renderPole(glm::vec3 position1, glm::vec3 position2)
{
	//create SPRING geometry
	std::vector<glm::vec3> Pole_verts;
	Pole_verts.push_back(position1);
	Pole_verts.push_back(position2);

	//spring
	glGenVertexArrays(1, &_massVertexArray);
	glBindVertexArray(_massVertexArray);

	glGenBuffers(1, &_massVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _massVertexBuffer);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * 2,
		Pole_verts.data(),
		GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	glBindVertexArray(_massVertexArray);

	glUseProgram(_programSpring);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, 3.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);

	_modelview = glm::lookAt(eye, center, up);

	glm::mat4 identity(1.0f);
	_projection = glm::perspective(45.0f, _aspectRatio, 0.01f, 100.0f);

	glm::mat4 rotationX = glm::rotate(identity, _xRot  * PI / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	_modelview *= rotationX;

	glm::mat4 rotationY = glm::rotate(identity, _yRot  * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	_modelview *= rotationY;

	glm::mat4 rotationZ = glm::rotate(identity, _zRot  * PI / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));

	_modelview *= rotationZ;

	//Uniform variables
	glUniformMatrix4fv(glGetUniformLocation(_programSpring, "modelview"), 1, GL_FALSE, glm::value_ptr(_modelview));
	glUniformMatrix4fv(glGetUniformLocation(_programSpring, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));
	glUniform3fv(glGetUniformLocation(_programSpring, "lightPosition"), 1, glm::value_ptr(lightPosition));
	glUniform3fv(glGetUniformLocation(_programSpring, "translate"), 1, glm::value_ptr(glm::vec3(0, 0, 0)));
	glUniform1f(glGetUniformLocation(_programSpring, "scale"), 1);
	glUniform3fv(glGetUniformLocation(_programSpring, "diffuse_albedo"), 1, glm::value_ptr(glm::vec3(0.9, 0.3, 0.3)));


	glLineWidth(10.0);
	glDrawArrays(GL_LINE_STRIP, 0, 2);

	glBindVertexArray(0);
}

void SceneShader::renderTriangle(glm::vec3 position1, glm::vec3 position2, glm::vec3 position3)
{
	//create SPRING geometry
	std::vector<glm::vec3> T_verts;
	T_verts.push_back(position1);
	T_verts.push_back(position2);
	T_verts.push_back(position3);
	
	//spring
	glGenVertexArrays(1, &_springVertexArray);
	glBindVertexArray(_springVertexArray);

	glGenBuffers(1, &_springVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _springVertexBuffer);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * 3,		
		T_verts.data(),
		GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glUseProgram(_programSpring);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, 15.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);

	_modelview = glm::lookAt(eye, center, up);

	glm::mat4 identity(1.0f);
	_projection = glm::perspective(45.0f, _aspectRatio, 0.01f, 100.0f);

	glm::mat4 rotationX = glm::rotate(identity, _xRot  * PI / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	_modelview *= rotationX;

	glm::mat4 rotationY = glm::rotate(identity, _yRot  * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	_modelview *= rotationY;

	glm::mat4 rotationZ = glm::rotate(identity, _zRot  * PI / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));

	_modelview *= rotationZ;

	//Uniform variables
	glUniformMatrix4fv(glGetUniformLocation(_programSpring, "modelview"), 1, GL_FALSE, glm::value_ptr(_modelview));
	glUniformMatrix4fv(glGetUniformLocation(_programSpring, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));
	glUniform3fv(glGetUniformLocation(_programSpring, "lightPosition"), 1, glm::value_ptr(lightPosition));
	glUniform3fv(glGetUniformLocation(_programSpring, "translate"), 1, glm::value_ptr(glm::vec3(5, 5, 0)));
	glUniform1f(glGetUniformLocation(_programSpring, "scale"), 1);
	glUniform3fv(glGetUniformLocation(_programSpring, "diffuse_albedo"), 1, glm::value_ptr(glm::vec3(0.0, 0.2, 1.0)));

	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

	glBindVertexArray(0);
}



void SceneShader::renderLight()
{
	glUseProgram(_programLight);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, 15.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);

	_modelview = glm::lookAt( eye, center, up);

	_projection = glm::perspective( 45.0f, _aspectRatio, 0.01f, 100.0f);

	glm::mat4 identity(1.0f);

	glm::mat4 rotationX = glm::rotate(identity, _xRot  * PI / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	_modelview *= rotationX;

	glm::mat4 rotationY = glm::rotate(identity, _yRot  * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	_modelview *= rotationY;

	glm::mat4 rotationZ = glm::rotate(identity, _zRot  * PI / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));

	_modelview *= rotationZ;

	//uniform variables
	glUniformMatrix4fv(glGetUniformLocation(_programLight, "modelview"), 1, GL_FALSE, glm::value_ptr(_modelview));
	glUniformMatrix4fv(glGetUniformLocation(_programLight, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));
	glUniform3fv(glGetUniformLocation(_programLight, "lightPosition"), 1, glm::value_ptr(lightPosition));

	glPointSize(20.0f);
	glDrawArrays( GL_POINTS, 0, 1);

}

void SceneShader::render()
{
	if (g_play) {
		t += dt;
		simulation(t);
	}
	
	renderPlane();
	renderPole(glm::vec3(0.05, 1.2, 0.0), glm::vec3(0.05, -0.85, 0.0));
	renderLight();

		
}

void SceneShader::setZTranslation(float z)
{
	_zTranslation = z;
}

void SceneShader::setAspectRatio(float ratio)
{
	_aspectRatio = ratio;
}

void SceneShader::setRotationX( float x )
{
	_xRot = x;
}

void SceneShader::setRotationY( float y )
{
	_yRot = y;
}

void SceneShader::setRotationZ(float z)
{
	_zRot = z;
}

void SceneShader::shutdown()
{
	glDeleteVertexArrays(1, &_massVertexArray);
	glDeleteVertexArrays(1, &_springVertexArray);
	glDeleteVertexArrays(1, &_planeVertexArray);
}

void SceneShader::updateLightPositionX(float x)
{
	lightPosition.x += x;
}

void SceneShader::updateLightPositionY(float y)
{
	lightPosition.y += y;
}

void SceneShader::updateLightPositionZ(float z)
{
	lightPosition.z += z;
}

void SceneShader::updateDisplacement(float y)
{
	Displacement += y;
	
}

void SceneShader::play()
{
	if(g_play == true)
		g_play = false;
	else
		g_play = true;
}

SceneShader::~SceneShader()
{
	shutdown();
}
