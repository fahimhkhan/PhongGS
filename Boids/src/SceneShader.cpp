/*
 * SceneShader.cpp
 * Created by: Fahim Hasan Khan
 * Date :10-Apr-2017
 *
 */

#include "SceneShader.h"
static float PI = 3.14159265359;
float camZ = 15.0f;

float t = 0.0;
float dt = 0.001;
float R = 50;
glm::vec3 G(0.0, -9.8, 0.0); //Gravity
glm::vec3 ObsP(-20.0f, -10.0f, -20.0f); //Obstacle Position

std::vector<glm::vec3> cverts;
std::vector<glm::vec3> tris;
std::vector<glm::vec3> verts;

int c;
int count = 1;

SceneShader::SceneShader(): Shader()
{
	_programPlane = 0;
	_programForce = 0;
	_programBoid = 0;
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
	lightPosition = glm::vec3(10.0, 25.0, -50.0);
	
	//Read parameter from file
	std::ifstream file("parameters.txt");
	std::string line;

	int p = 0;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string result;
		while (std::getline(iss, result, ','))
		{
			parameter[p] = atof(result.c_str());
			p++;
		}
	}


	g_play = true;
	size = parameter[0];
	R = parameter[1];
	dt = parameter[2];
	nBoid = size*size;

	P = (Boid*)malloc((nBoid+1)*sizeof(Boid));
	Sp = (Force*)malloc((nBoid*6)*sizeof(Force));

	//INITIALIZE
	for (int i = 0; i < size; i++){
		for (int j = 0; j < size; j++){
			int a = (size*i) + j;

			float x = (j - size * 0.1);
			float y = (i - size * 0.1);
			P[a].x = glm::vec3(x, 0.0, -y);
			P[a].v = glm::vec3(0.0, 0.0, 0.0);
			P[a].F = glm::vec3(0.0, 0.0, 0.0);
			//P[a].M = glm::vec3(0.0, 0.0, 0.0);
			P[a].m = 0.5;
		}
	}

	c = (size*(size/2)) + size/2;

	P[c].m = 0.0; //fixing position
	
	//CREATE NEIGHBORHOOD CONNECTION
	float dmin = 1.0;
	

	for (int i = 0; i < nBoid; i++){
		for (int j = 0; j < nBoid; j++){
			if (i < j){
				float d = glm::distance(P[i].x, P[j].x);
				if (d <= dmin){
					Sp[nForce].id1 = i;
					Sp[nForce].id2 = j;
					Sp[nForce].l = d;
					nForce++;
				}
			}
		}
	}

	//printf("\n%d",nForce);
}

void SceneShader::simulation(float t) {

	//Accumulate Cohession
	float k1 = 10000.0;
	float d1 = 0.5;		

	for (int i = 0; i < nForce; i++){
		
		int si = Sp[i].id1;
		int sj = Sp[i].id2;

		float ds = glm::distance(P[si].x, P[sj].x);
		glm::vec3 Fs = -k1*(ds - d1)*((P[si].x - P[sj].x) / ds);

		P[si].F = P[si].F + Fs;
		P[sj].F = P[sj].F - Fs;
	}


	//Accumulate Other Behavioral forces
	float ra = 0.05, rc = 0.1, rg = 0.15;
	
	glm::vec3 F = glm::vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < nBoid; i++){
		for (int j = 0; j < nBoid; j++){
			if (i < j){
				//Avoid
				float d = glm::distance(P[i].x, P[j].x);

				if (d < ra){
					//P[j].x = glm::vec3(P[j].x.x + 0.05, P[j].x.y + 0.05, P[j].x.z + 0.05);
					float r = d / ra;
					float fw = (pow((1 - r), 3))*(3 * r + 1);							//Wendland
					F = fw * ((P[i].x - P[j].x) / d);
					//printf("\nCollision Avoided");
				}

				//Cohession (Already applied in the previous for loop)
				else if (d < rc){
					F = glm::vec3(0.0, 0.0, 0.0);
				}

				//Gathering
				else if (d < rg){
					//F = glm::vec3(0.0, 0.0, 0.0);
					float r = d / ra;
					float fw = (pow((1 - r), 3))*(3 * r + 1);							//Wendland
					F = -fw * ((P[i].x - P[j].x) / d);
					//printf("\nGathering force Applied");
				}
				else{
					F = glm::vec3(0.0, 0.0, 0.0);
				}
				P[i].F = P[i].F + F;
				P[j].F = P[j].F - F;
			}
		}
	}
	
	//Accumulate other forces
	for (int i = 0; i < nBoid; i++){
		glm::vec3 Fg = G*P[i].m;
		P[i].F = P[i].F + Fg;
	}



	//For each Boid
	//Semi-implicit Eular integration
	for (int i = 0; i < nBoid; i++){
		if ((P[i].m) > 0.0){								//If Boid
			
			//UPDATE VELOCITY
			P[i].v = P[i].v + ((P[i].F) / (P[i].m))*dt;	

			//UPDATE POSITION
			P[i].x = P[i].x + dt*P[i].v;
			
			//UPDATE ORIENTATION
			glm::vec3 T = normalize(P[i].v); //Tangent

			glm::vec3 A = P[i].F - (glm::dot(T, P[i].F))*T;
			glm::vec3 tmp = A - G;
			glm::vec3 N1 = normalize(tmp); 

			glm::vec3 B = glm::cross(N1, T); //Binormal

			glm::vec3 N = glm::cross(T, B); //Normal

			P[i].M = glm::orientation(normalize(N), normalize(B));
		}

		else if ((P[i].m) == 0.0){							//If Automonous Drone
			//P[i].x = glm::vec3(0.0, 0.0, 0.0);
			float x = (std::cos(t*2))*25;
			float z = (std::sin(t*2))*25;
			//printf("\n%f%f", x, z);
			P[i].x = glm::vec3(x, 0.0, z);
		}

		else if ((P[i].m) < 0.0){							//If Mouse Controlled Drone
			P[i].x = P[i].x;
		}

		P[i].F = glm::vec3(0.0, 0.0, 0.0);
	}

	//Obstacle Avoidance
	for (int i = 0; i < nBoid; i++){
		float d1 = glm::distance(P[i].x, ObsP);
		//if (d1 <3.5)
		//	F = (1/(d1*d1)) * ((P[i].x - ObsP) / d1);
		//P[i].F = P[i].F + F;
		while ((d1>0) && (d1 < 5))
			P[i].x = P[i].x + (glm::vec3(0.1, 0.0, 0.1));
	}

	//Render
	for (int i = 0; i < nBoid; i++){
		if (i != c){
			renderBoid(P[i].x, P[i].M, glm::vec3(0.0, 0.0, 1.0));
		}
		else{
			if (P[i].m == 0.0)
				renderLight(P[i].x, glm::vec3(1.0, 0.0, 0.0), 30.0f);
			else
				renderLight(P[i].x, glm::vec3(0.0, 1.0, 0.0), 30.0f);
		}
	}

}


void SceneShader::createVertexBuffer()
{
	//create plane geometry
	
	float R = 50, cx, cz, cr;

	cr = (2.0f * PI * R);
	for (int i = 0; i <= int(cr); i++)
	{
		float theta = 2.0f * PI * float(i) / float(cr);
		float cx = R * (std::cos(theta));
		float cy = R * (std::sin(theta));
		cverts.push_back(glm::vec3(cx, 0.0f, cy));
		cverts.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
		count=count+2;
	}
	cverts.push_back(glm::vec3(R * cos(0), 0.0f, R * sin(0)));

	//create Boid geometry
	verts.push_back(glm::vec3(0.0f, 0.0f, -0.1f));
	verts.push_back(glm::vec3(0.1f, 0.0f, 0.0f));
	verts.push_back(glm::vec3(0.2f, 0.0f, 0.0f));
	verts.push_back(glm::vec3(0.0f, 0.0f, 0.1f));
	verts.push_back(glm::vec3(0.2f, 0.1f, 0.0f));
	verts.push_back(glm::vec3(0.1f, 0.0f, 0.0f));
	verts.push_back(glm::vec3(0.0f, 0.0f, -0.1f));

	//Create Obstracle (Pyramid) Geometry
	tris.push_back(glm::vec3(-3.0f, 0.0f, 0.0f));
	tris.push_back(glm::vec3(0.0f, 15.0f, 0.0f));
	tris.push_back(glm::vec3(0.0f, 0.0f, -3.0f));
	tris.push_back(glm::vec3(0.0f, 15.0f, 0.0f));
	tris.push_back(glm::vec3(3.0f, 0.0f, 0.0f));
	tris.push_back(glm::vec3(0.0f, 15.0f, 0.0f));
	tris.push_back(glm::vec3(0.0f, 0.0f, 3.0f));
	tris.push_back(glm::vec3(0.0f, 15.0f, 0.0f));
	tris.push_back(glm::vec3(-3.0f, 0.0f, 0.0f));
	
	//passing model attributes to the GPU
	//plane
	glGenVertexArrays(1, &_planeVertexArray);
	glBindVertexArray(_planeVertexArray);

	glGenBuffers(1, &_planeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _planeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * count,
		cverts.data(),
		GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	//Obstracle
	glGenVertexArrays(1, &_ObsVertexArray);
	glBindVertexArray(_ObsVertexArray);

	glGenBuffers(1, &_ObsVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _ObsVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * 9,
		tris.data(),
		GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	//boid
	glGenVertexArrays(1, &_boidVertexArray);
	glBindVertexArray(_boidVertexArray);

	glGenBuffers(1, &_boidVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _boidVertexBuffer);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * 7,
		verts.data(),
		GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

}


void SceneShader::startup()
{
	
	_programPlane = compile_shaders("./shaders/plane.vert", "./shaders/plane.frag");
	_programBoid = compile_shaders("./shaders/boid.vert", "./shaders/boid.frag");
	_programLight = compile_shaders("./shaders/light.vert", "./shaders/light.frag");

	createVertexBuffer();

}

void SceneShader::renderPlane()
{
	glBindVertexArray(_planeVertexArray);

	glUseProgram(_programPlane);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, camZ);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);

	_modelview = glm::lookAt( eye, center, up);

	glm::mat4 identity(1.0f);
	_projection = glm::perspective( 45.0f, _aspectRatio, 0.01f, 100.0f);

	glm::mat4 rotationX = glm::rotate(identity, _xRot  * PI / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	_modelview *= rotationX;

	glm::mat4 rotationY = glm::rotate(identity, _yRot  * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	_modelview *= rotationY;

	//Uniform variables
	glUniformMatrix4fv(glGetUniformLocation(_programPlane, "modelview"), 1, GL_FALSE, glm::value_ptr(_modelview));
	glUniformMatrix4fv(glGetUniformLocation(_programPlane, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));
	glUniform3fv(glGetUniformLocation(_programPlane, "inColor"), 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f)));
	glUniform3fv(glGetUniformLocation(_programPlane, "lightPosition"), 1, glm::value_ptr(lightPosition) );

	glDrawArrays(GL_TRIANGLE_STRIP, 0, count);

	glBindVertexArray(0);
}

void SceneShader::renderObs(glm::vec3 init_position)
{
	glBindVertexArray(_ObsVertexArray);

	glUseProgram(_programBoid);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, camZ);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);

	_modelview = glm::lookAt(eye, center, up);

	glm::mat4 identity(1.0f);
	_projection = glm::perspective(45.0f, _aspectRatio, 0.01f, 100.0f);

	glm::mat4 rotationX = glm::rotate(identity, _xRot  * PI / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	_modelview *= rotationX;

	glm::mat4 rotationY = glm::rotate(identity, _yRot  * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	_modelview *= rotationY;

	//Uniform variables
	glUniformMatrix4fv(glGetUniformLocation(_programBoid, "modelview"), 1, GL_FALSE, glm::value_ptr(_modelview));
	glUniformMatrix4fv(glGetUniformLocation(_programBoid, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));
	glUniform3fv(glGetUniformLocation(_programBoid, "lightPosition"), 1, glm::value_ptr(lightPosition));
	glUniform3fv(glGetUniformLocation(_programBoid, "translate"), 1, glm::value_ptr(glm::vec3(init_position.x, init_position.y, init_position.z)));
	glUniform1f(glGetUniformLocation(_programBoid, "scale"), 1);
	glUniformMatrix4fv(glGetUniformLocation(_programBoid, "rotation"), 1, GL_FALSE, glm::value_ptr(glm::orientation(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f))));
	glUniform3fv(glGetUniformLocation(_programBoid, "diffuse_albedo"), 1, glm::value_ptr(glm::vec3(0.8f, 0.734f, 0.563f)));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 9);

	glBindVertexArray(0);
}



void SceneShader::renderBoid(glm::vec3 init_position, glm::mat4 orientation, glm::vec3 color)
{
	glBindVertexArray(_boidVertexArray);

	glUseProgram(_programBoid);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, camZ);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);

	_modelview = glm::lookAt(eye, center, up);

	glm::mat4 identity(1.0f);
	_projection = glm::perspective(45.0f, _aspectRatio, 0.01f, 100.0f);

	//_modelview *= rotation;
	
	glm::mat4 rotationX = glm::rotate(identity, _xRot  * PI / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	_modelview *= rotationX;

	glm::mat4 rotationY = glm::rotate(identity, _yRot  * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	_modelview *= rotationY;

	//Uniform variables
	glUniformMatrix4fv(glGetUniformLocation(_programBoid, "modelview"), 1, GL_FALSE, glm::value_ptr(_modelview));
	glUniformMatrix4fv(glGetUniformLocation(_programBoid, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));
	glUniformMatrix4fv(glGetUniformLocation(_programBoid, "rotation"), 1, GL_FALSE, glm::value_ptr(orientation));
	glUniform3fv(glGetUniformLocation(_programBoid, "lightPosition"), 1, glm::value_ptr(lightPosition));
	glUniform3fv(glGetUniformLocation(_programBoid, "translate"), 1, glm::value_ptr(glm::vec3(init_position.x, init_position.y, init_position.z)));
	glUniform1f(glGetUniformLocation(_programBoid, "scale"), 1);
	glUniform3fv(glGetUniformLocation(_programBoid, "diffuse_albedo"), 1, glm::value_ptr(color));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 7);

	glBindVertexArray(0);
}


void SceneShader::renderLight(glm::vec3 init_position, glm::vec3 color, float rp)
{
	glUseProgram(_programLight);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, camZ);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);

	_modelview = glm::lookAt( eye, center, up);

	_projection = glm::perspective( 45.0f, _aspectRatio, 0.01f, 100.0f);

	glm::mat4 identity(1.0f);

	glm::mat4 rotationX = glm::rotate(identity, _xRot  * PI / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	_modelview *= rotationX;

	glm::mat4 rotationY = glm::rotate(identity, _yRot  * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	_modelview *= rotationY;

	//uniform variables
	glUniformMatrix4fv(glGetUniformLocation(_programLight, "modelview"), 1, GL_FALSE, glm::value_ptr(_modelview));
	glUniformMatrix4fv(glGetUniformLocation(_programLight, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));
	glUniform3fv(glGetUniformLocation(_programLight, "lightPosition"), 1, glm::value_ptr(init_position));
	glUniform3fv(glGetUniformLocation(_programLight, "inColor"), 1, glm::value_ptr(color));

	glPointSize(rp);
	glDrawArrays( GL_POINTS, 0, 1);

}

void SceneShader::render()
{
	renderPlane();
	renderObs(ObsP);
	renderLight(lightPosition, glm::vec3(1.0f, 1.0f, 1.0f), 50.0f);
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
	glDeleteVertexArrays(1, &_boidVertexArray);
	glDeleteVertexArrays(1, &_ObsVertexArray);
	glDeleteVertexArrays(1, &_planeVertexArray);
}

void SceneShader::updateLightPositionX(float x)
{
	P[c].x.x += x;
	P[c].m = -1.0;
}

void SceneShader::updateLightPositionY(float y)
{
	//P[c].x.y += y;
	return;
}

void SceneShader::updateLightPositionZ(float z)
{
	P[c].x.z += z;
	P[c].m = -1.0;
}

void SceneShader::updateDisplacement(float y)
{
	camZ += y;
	
}

void SceneShader::setGXY(float Gx, float Gy)
{
	P[c].x = glm::vec3(P[c].x.x + Gy, 0, P[c].x.z + Gx);
	//printf("\n%f,%f", Gx, Gy);
	P[c].m = -1.0;
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
