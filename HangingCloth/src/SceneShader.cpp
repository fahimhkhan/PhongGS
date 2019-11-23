/*
 * SceneShader.cpp
 * Created by: Fahim Hasan Khan
 * Date :23-Mar-2017
 *
 */

#include "SceneShader.h"
static float PI = 3.14159265359;
float t = 0.0;
float dt = 0.001;
float ks = 200.0;	//Spring stiffness
float kd = 0.5;		//Damping coefficient
float l = 1.0;		//Rest length of spring
glm::vec3 G(0.0, -9.8, 0.0); //Gravity
glm::vec3 Fs(0.0, 0.0, 0.0);

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
	lightPosition = glm::vec3(5.0, 5.0, 5.0);
	g_play = true;
	size = 10;
	nMass = size*size;
	nSpring = 0;

	P = (Mass*)malloc((nMass+1)*sizeof(Mass));
	Sp = (Spring*)malloc((nMass*6)*sizeof(Spring));

	//INITIALIZE
	float H = 4.5;
	for (int i = 0; i < size; i++){
		for (int j = 0; j < size; j++){
			//P[i].x = glm::vec3((i*1.0), (H - (i*0.0001)), 0.0);
			int a = (size*i) + j;

			float x = (j - size * 0.5);
			float y = (i - size * 0.5);
			P[a].x = glm::vec3(x, -y, 0.0);
			P[a].v = glm::vec3(0.0, 0.0, 0.0);
			P[a].F = glm::vec3(0.0, 0.0, 0.0);
			P[a].m = 1.0;
		}
	}

	
	P[0].m = 0.0; //fixing position
	P[size-1].m = 0.0; //fixing position
	
	//CREATE SPRINGS
	float lmin = 1.5;
	

	for (int i = 0; i < nMass; i++){
		for (int j = 0; j < nMass; j++){
			if (i < j){
				float d = glm::distance(P[i].x, P[j].x);
				if (d <= lmin){
					Sp[nSpring].id1 = i;
					Sp[nSpring].id2 = j;
					Sp[nSpring].l = d;
					nSpring++;
				}
			}
		}
	}

	printf("\n%d",nSpring);
}

void SceneShader::simulation(float t) {

	//Accumulate spring forces on particles
	for (int i = 0; i < nSpring; i++){
		
		int si = Sp[i].id1;
		int sj = Sp[i].id2;
		Fs = -ks*((glm::distance(P[si].x,P[sj].x)) - l)*((P[si].x - P[sj].x)/glm::distance(P[si].x,P[sj].x));

		glm::vec3 Fd = -kd*P[si].v;

		P[si].F = P[si].F + Fs+Fd;
		P[sj].F = P[sj].F - Fs-Fd;
	}


	//Accumulate other forces
	for (int i = 0; i < nMass; i++){
		glm::vec3 Fg = G*P[i].m;
		P[i].F = P[i].F + Fg;
	}
	

	//Semi-implicit Eular integration
	for (int i = 0; i < nMass; i++){
		if ((P[i].m) > 0){							//If not fixed

			P[i].v = P[i].v + ((P[i].F) / (P[i].m))*dt;			//using real force
			
			P[i].x = P[i].x + dt*P[i].v;
			//printf("\nP[i].F = %f >>>> P[i].v =%f,%f,%f", P[i].F.y, P[i].v.x, P[i].v.y, P[i].v.z);
		}

		P[i].F = glm::vec3(0.0, 0.0, 0.0);
	}

	//RENDER
	for (int i = 0; i < nSpring; i++){
		renderSpring(P[Sp[i].id1].x, P[Sp[i].id2].x);
	}



	int s = size;
	auto id = [s](int r, int c) { return (s)*r + c; };
	int Tcount = 0;
	for (int row = 0; row < size - 1; ++row) {
		for (int col = 0; col < size - 1; ++col) {
			int a = id(row, col);
			int b = id(row, col + 1);
			int c = id(row + 1, col);
			int d = id(row + 1, col + 1);

			tris.push_back(glm::vec3(a, b, c));
			tris.push_back(glm::vec3(c, b, d));
			Tcount = Tcount + 2;
		}
	}

	for (int i = 0; i<Tcount; i++)
		renderTriangle(P[(int)(tris[i].x)].x, P[(int)(tris[i].y)].x, P[(int)(tris[i].z)].x);

}

void SceneShader::CreateSprings(int lmin){
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
	glm::vec3 eye(0.0f, 0.3f, 10.0f);
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

void SceneShader::renderSpring(glm::vec3 position1, glm::vec3 position2)
{
	//create SPRING geometry
	std::vector<glm::vec3> spring_verts;
	spring_verts.push_back(position1);
	spring_verts.push_back(position2);
	
	//spring
	glGenVertexArrays(1, &_springVertexArray);
	glBindVertexArray(_springVertexArray);

	glGenBuffers(1, &_springVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _springVertexBuffer);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * 2,		
		spring_verts.data(),
		GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	
	glBindVertexArray(_springVertexArray);

	glUseProgram(_programSpring);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, 10.0f);
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
	glUniform3fv(glGetUniformLocation(_programSpring, "diffuse_albedo"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 1.0)));

	
	glLineWidth(5.0);
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
	glGenVertexArrays(1, &_massVertexArray);
	glBindVertexArray(_massVertexArray);

	glGenBuffers(1, &_massVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _massVertexBuffer);

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
	glUniform3fv(glGetUniformLocation(_programSpring, "translate"), 1, glm::value_ptr(glm::vec3(0, 0, 0)));
	glUniform1f(glGetUniformLocation(_programSpring, "scale"), 1.5);
	glUniform3fv(glGetUniformLocation(_programSpring, "diffuse_albedo"), 1, glm::value_ptr(glm::vec3(1.0, 0.0, 0.0)));


	glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

	glBindVertexArray(0);
}

void SceneShader::renderLight()
{
	glUseProgram(_programLight);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, 10.0f);
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
	//renderPlane();
	renderLight();
	if (g_play) {
		t += dt;
		simulation(t);
	}

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
