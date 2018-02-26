/* Lab 6 base code - transforms using local matrix functions
	to be written by students -
	based on lab 5 by CPE 471 Cal Poly Z. Wood + S. Sueda
	& Ian Dunn, Christian Eckhardt

	Modified heavily by Anthony Delgado for the
	implementation of a 3D version of the 1979 arcade classic, "Asteroids"
*/
#include <iostream>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "camera.h"
// used for helper in perspective
#include "glm/glm.hpp"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "asteroid.hpp"
#include "ship.hpp"
#include "bullet.hpp"
#include "controller.h"
#include "BillboardData.h"
#include "BillboardFile.h"
#include <set>

using namespace std;
using namespace glm;
double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}
class character
{
public:
	vec2 pos,impulse;
	int lifes;
	character()
	{
		pos = vec2(-20, 0);
		impulse = vec2(0, 0);
	}
	void process(float ftime)
	{
		pos += impulse * ftime;
		impulse.y -= 10* ftime;
		if (pos.y < -1)
		{
			pos.y = -1;
			impulse.y = 0;
		}
		
	}
};
character player;
class Billboard {
public:
	GLuint VAO;
	GLuint VBO, TexBuffer;
	GLuint Texture;
	shared_ptr<Program> *prog;
	GLfloat *data;

	glm::mat4 M;
	glm::vec3 position, speed;

	float z;
	float frame;
	
		void init( BillboardData *bbd)
	{
		
			
		//generate the VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VBO);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		//actually memcopy the data - only do this once
		data = new GLfloat[12];
		int verc = 0;
		data[verc++] = bbd->points[0][0], data[verc++] = bbd->points[0][1];
		data[verc++] = bbd->points[1][0], data[verc++] = bbd->points[1][1];
		data[verc++] = bbd->points[2][0], data[verc++] = bbd->points[2][1];
		data[verc++] = bbd->points[3][0], data[verc++] = bbd->points[3][1];
		data[verc++] = bbd->points[4][0], data[verc++] = bbd->points[4][1];
		data[verc++] = bbd->points[5][0], data[verc++] = bbd->points[5][1];
		glBufferData(GL_ARRAY_BUFFER, bbd->points.size() * sizeof(vec3), &bbd->points[0], GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &TexBuffer);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, TexBuffer);
		
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, bbd->texcoords.size() * sizeof(vec2), &bbd->texcoords[0], GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(2);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


		int width, height, channels;
		char filepath[1000];
		cout << bbd->texture.c_str() << endl;
		strcpy(filepath, bbd->texture.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		/*GLuint TexLocation = glGetUniformLocation((*prog)->pid, "tex");

		glUseProgram((*prog)->pid);
		glUniform1i(TexLocation, 0);*/
	}
	void init(shared_ptr<Program> *program, float zposition, string imageFile)
	{
		prog = program;
		z = zposition;

		//generate the VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VBO);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		GLfloat *ver = new GLfloat[18];
		int verc = 0;
		ver[verc++] = -1.0, ver[verc++] = -1.0, ver[verc++] = 0.0;
		ver[verc++] = 1.0, ver[verc++] = -1.0, ver[verc++] = 0.0;
		ver[verc++] = -1.0, ver[verc++] = 1.0, ver[verc++] = 0.0;
		ver[verc++] = 1.0, ver[verc++] = -1.0, ver[verc++] = 0.0;
		ver[verc++] = 1.0, ver[verc++] = 1.0, ver[verc++] = 0.0;
		ver[verc++] = -1.0, ver[verc++] = 1.0, ver[verc++] = 0.0;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), ver, GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &TexBuffer);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, TexBuffer);
		GLfloat *cube_tex = new GLfloat[12];
		int texc = 0;
		cube_tex[texc++] = 0, cube_tex[texc++] = 0;
		cube_tex[texc++] = 1, cube_tex[texc++] = 0;
		cube_tex[texc++] = 0, cube_tex[texc++] = 1;
		cube_tex[texc++] = 1, cube_tex[texc++] = 0;
		cube_tex[texc++] = 1, cube_tex[texc++] = 1;
		cube_tex[texc++] = 0, cube_tex[texc++] = 1;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), cube_tex, GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(2);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


		int width, height, channels;
		char filepath[1000];
		strcpy(filepath, imageFile.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		GLuint TexLocation = glGetUniformLocation((*prog)->pid, "tex");

		glUseProgram((*prog)->pid);
		glUniform1i(TexLocation, 0);
	}

	void draw(shared_ptr<Program> *program, bool depthTest, float x, float y)
	{
		if (!depthTest)
			glDisable(GL_DEPTH_TEST);
		glBindVertexArray(VAO);

		M = glm::translate(glm::mat4(1), glm::vec3(x, y, z-10));
		glUniformMatrix4fv((*program)->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
	}
};

vector<Billboard> bill;
Billboard sprite;

class Application : public EventCallbacks
{
	typedef std::set<Asteroid *> Asteroids;
	typedef std::set<Bullet *> Bullets;
public:
	Asteroids asteroids;
	Bullets bullets;
	shared_ptr<Ship> ship;
	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> prog2,laserprog,shipprog,billprog,scoreprog;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape;
	shared_ptr<Shape> shape2,lasershape,sun;

	//camera
	camera mycam;

	//texture for sim
	GLuint Texture;
	GLuint Texture2,Texture3,Texture4,Texture5,TextureExplosion;
	GLuint TextureScore;

	// Contains vertex information for OpenGL
	GLuint VertexArrayIDBox;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferIDBox,VertexBufferTex;

	

	int points = 0;
	float lasershot = 0;
	float destroyed = 5;
	vec2 offset = vec2(0, 0);
	vec2 dig1 = vec2(0.7, 0.1);
	vec2 dig2 = vec2(0.7, 0.1);
	vec2 dig3 = vec2(0.7, 0.1);

	CXBOXController *gamepad = new CXBOXController(1); //1 would be the only one or the fist one of max 4 controller

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{


		GLSL::checkVersion();

		
		// Set background color.
		glClearColor(0.12f, 0.34f, 0.56f, 1.0f);

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		//culling:
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		//transparency
		glEnable(GL_BLEND);

		//next function defines how to mix the background color with the transparent pixel in the foreground. 
		//This is the standard:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		if (! prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
		prog->addAttribute("vertTan");
		prog->addAttribute("vertBinorm");

		prog2 = make_shared<Program>();
		prog2->setVerbose(true);
		prog2->setShaderNames(resourceDirectory + "/vert.glsl", resourceDirectory + "/frag_nolight.glsl");
		if (!prog2->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog2->init();
		prog2->addUniform("P");
		prog2->addUniform("V");
		prog2->addUniform("M");
		prog2->addAttribute("vertPos");
		prog2->addAttribute("vertTex");


		shipprog = make_shared<Program>();
		shipprog->setVerbose(true);
		shipprog->setShaderNames(resourceDirectory + "/shipvert.glsl", resourceDirectory + "/shipfrag.glsl");
		if (!shipprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		shipprog->init();
		shipprog->addUniform("P");
		shipprog->addUniform("V");
		shipprog->addUniform("M");
		shipprog->addAttribute("vertPos");
		shipprog->addAttribute("vertNor");
		shipprog->addAttribute("vertTex");
		shipprog->addUniform("campos");

		laserprog = make_shared<Program>();
		laserprog->setVerbose(true);
		laserprog->setShaderNames(resourceDirectory + "/laservert.glsl", resourceDirectory + "/laserfrag.glsl");
		laserprog->init();
		if (!laserprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		laserprog->addUniform("P");
		laserprog->addUniform("V");
		laserprog->addUniform("M");
		laserprog->addAttribute("vertPos");

		billprog = make_shared<Program>();
		billprog->setVerbose(true);
		billprog->setShaderNames(resourceDirectory + "/billvert.glsl", resourceDirectory + "/billfrag.glsl");
		if (!billprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		billprog->init();
		billprog->addUniform("P");
		billprog->addUniform("V");
		billprog->addUniform("offset");
		billprog->addUniform("vpws");
		billprog->addAttribute("vertPos");
		billprog->addAttribute("vertTex");

		scoreprog = make_shared<Program>();
		scoreprog->setVerbose(true);
		scoreprog->setShaderNames(resourceDirectory + "/scorevert.glsl", resourceDirectory + "/scorefrag.glsl");
		if (!billprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		scoreprog->init();
		scoreprog->addUniform("P");
		scoreprog->addUniform("V");
		scoreprog->addUniform("M");
		scoreprog->addUniform("offset");
		scoreprog->addAttribute("vertPos");
		scoreprog->addAttribute("vertTex");

		
	}



	void initGeom(const std::string& resourceDirectory) 
	{

		sprite.init(&scoreprog, 3, resourceDirectory + "/mario.png");
		BillboardFile test = BillboardFile(resourceDirectory + "/level2.grl");
		vector<BillboardData> things = test.getAll();
		bill.resize(things.size());
		for (int ii = 0; ii < things.size(); ii++)
		{
			bill[ii].init(&things[ii]);
		}
	}
	void render()
	{
		double ftime = get_last_elapsed_time();
		if (gamepad->IsConnected())
		{
			if (gamepad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_Y)
				mycam.w = 1;
			else
				mycam.w = 0;
			if (gamepad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_A)
			{

				if (player.impulse.y == 0.0)
					player.impulse.y = 10;
			}
			

			SHORT lx = gamepad->GetState().Gamepad.sThumbLX;
			SHORT ly = gamepad->GetState().Gamepad.sThumbLY;

			if (lx > 3000 || lx < 3000)
			{
				float nx = (float)lx / 32000.0;
				player.impulse.x = nx*4;

			}
			else
				player.impulse.x = 0;
		

		}

		for (int ii = 1; ii < 2; ii++)
		{

			/*player.pos = collide(player.pos, bill.data);*/
			//	if (sprite.z == bill[ii].z) {
			float minX = bill[ii].data[0];
			float maxX = bill[ii].data[4];
			float minY = bill[ii].data[1];
			float maxY = bill[ii].data[5];

			if(inSquare(vec2(player.pos.x - 1, player.pos.y), minX, maxX, minY, maxY))
			{
				player.pos.x = maxX + 1;
			}

			if (inSquare(vec2(player.pos.x + 1, player.pos.y), minX, maxX, minY, maxY))
			{
				player.pos.x = minX - 1;
			}

			if (inSquare(vec2(player.pos.x, player.pos.y - 1), minX, maxX, minY, maxY))
			{
				player.pos.y = maxY + 1;
				player.impulse.y = 0;
			}

			if (inSquare(vec2(player.pos.x, player.pos.y + 1), minX, maxX, minY, maxY))
			{
				player.pos.y = minY - 1;
			}
		}
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		auto P = std::make_shared<MatrixStack>();
		P->pushMatrix();	
		P->perspective(70., width, height, 0.1, 1000.0f);
		glm::mat4 M,V,R,Sc;		
		static float angle = 0;
		angle += 0.01;
		static vec3 eCord(0,0,0);
					
		//VIEW MATRIX
		V = mycam.process();
				
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		scoreprog->bind();

		bool depth = true;

		//S
		M = glm::translate(mat4(1), glm::vec3(0,0,-3));
		glUniformMatrix4fv(scoreprog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(scoreprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(scoreprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);

		vec2 scorecoord = vec2(0, 0);
		glUniform2fv(scoreprog->getUniform("offset"), 1, &scorecoord.x);

		glActiveTexture(GL_TEXTURE0);
		

		for(int ii=1;ii<2;ii++)
			bill[ii].draw(&scoreprog,depth,scorecoord.x,scorecoord.y);


		player.process(ftime);

		sprite.draw(&scoreprog, depth, player.pos.x, player.pos.y);
		scoreprog->unbind();

	}

	vec2 collide(vec2 p, GLfloat *data)
	{
		vec2 v1 = vec2(data[0], data[1]);
		vec2 v2 = vec2(data[2], data[3]);
		vec2 v3 = vec2(data[4], data[5]);
		vec2 v4 = vec2(data[10], data[11]);

		vec2 r = p;

		if (isInTriangle(r.x + 1, r.y + 1, v1, v3, v4))
		{
			r = vec2(r.x - 1, r.y);
		}

		if (isInTriangle(r.x + 1, r.y + 1, v1, v2, v3))
		{
			r = vec2(r.x, r.y - 1);
		}

		if (isInTriangle(r.x + 1, r.y - 1, v1, v3, v4))
		{
			r = vec2(r.x - 1, r.y);
		}

		if (isInTriangle(r.x + 1, r.y + 1, v1, v2, v3))
		{
			r = vec2(r.x, r.y - 1);
		}

		return vec2(0, 0);
	}

	bool isInTriangle(float x, float y, vec2 a, vec2 b, vec2 c) 
	{
		return false;
	}

	bool inSquare(vec2 p, float minX, float maxX, float minY, float maxY)
	{
		return p.x > minX && p.x < maxX && p.y > minY && p.y < maxY;
	}

};
//*********************************************************************************************************
int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}


	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	
	
	WindowManager *windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}



void GetTangent(
	glm::vec3 *posA, glm::vec3 *posB, glm::vec3 *posC,
	glm::vec2 *texA, glm::vec2 *texB, glm::vec2 *texC,
	glm::vec3 *nnA, glm::vec3 *nnB, glm::vec3 *nnC,
	glm::vec3 *tanA, glm::vec3 *tanB, glm::vec3 *tanC);

//calculate tangents and binormals
void computeTangentSpace(Shape *shape)
{
	GLfloat* tangents = new GLfloat[shape->posBuf.size()]();
	GLfloat* binormals = new GLfloat[shape->posBuf.size()]();

	std::vector<glm::vec3 > tangent;
	std::vector<glm::vec3 > binormal;
	int im = 0;

	for (unsigned int i = 0; i < shape->eleBuf.size(); i = i + 3) {

		if (shape->eleBuf.at(i + 0) > im)			im = shape->eleBuf.at(i + 0);
		if (shape->eleBuf.at(i + 1) > im)			im = shape->eleBuf.at(i + 1);
		if (shape->eleBuf.at(i + 2) > im)			im = shape->eleBuf.at(i + 2);
		


		glm::vec3 vertex0 = glm::vec3(shape->posBuf.at(shape->eleBuf.at(i)*3),		shape->posBuf.at(shape->eleBuf.at(i) * 3 + 1), shape->posBuf.at(shape->eleBuf.at(i) * 3 + 2));
		glm::vec3 vertex1 = glm::vec3(shape->posBuf.at(shape->eleBuf.at(i + 1) * 3),	shape->posBuf.at(shape->eleBuf.at(i + 1) * 3 + 1), shape->posBuf.at(shape->eleBuf.at(i + 1) * 3 + 2));
		glm::vec3 vertex2 = glm::vec3(shape->posBuf.at(shape->eleBuf.at(i + 2) * 3),	shape->posBuf.at(shape->eleBuf.at(i + 2) * 3 + 1), shape->posBuf.at(shape->eleBuf.at(i + 2) * 3 + 2));

		glm::vec3 normal0 = glm::vec3(shape->norBuf.at(shape->eleBuf.at(i) * 3),		shape->norBuf.at(shape->eleBuf.at(i) * 3 + 1),		shape->norBuf.at(shape->eleBuf.at(i) * 3 + 2));
		glm::vec3 normal1 = glm::vec3(shape->norBuf.at(shape->eleBuf.at(i + 1) * 3),	shape->norBuf.at(shape->eleBuf.at(i + 1) * 3 + 1),	shape->norBuf.at(shape->eleBuf.at(i + 1) * 3 + 2));
		glm::vec3 normal2 = glm::vec3(shape->norBuf.at(shape->eleBuf.at(i + 2) * 3),	shape->norBuf.at(shape->eleBuf.at(i + 2) * 3 + 1),	shape->norBuf.at(shape->eleBuf.at(i + 2) * 3 + 2));

		glm::vec2 tex0 = glm::vec2(shape->texBuf.at(shape->eleBuf.at(i) * 2),			shape->texBuf.at(shape->eleBuf.at(i) * 2 + 1));
		glm::vec2 tex1 = glm::vec2(shape->texBuf.at(shape->eleBuf.at(i + 1) * 2),		shape->texBuf.at(shape->eleBuf.at(i + 1) * 2 + 1));
		glm::vec2 tex2 = glm::vec2(shape->texBuf.at(shape->eleBuf.at(i + 2) * 2),		shape->texBuf.at(shape->eleBuf.at(i + 2) * 2 + 1));

		glm::vec3 tan0, tan1, tan2; // tangents
		glm::vec3 bin0, bin1, bin2; // binormal

	
		GetTangent(&vertex0, &vertex1, &vertex2, &tex0, &tex1, &tex2, &normal0, &normal1, &normal2, &tan0, &tan1, &tan2);
		
		
		bin0 = glm::normalize(glm::cross(tan0, normal0));
		bin1 = glm::normalize(glm::cross(tan1, normal1));
		bin2 = glm::normalize(glm::cross(tan2, normal2));

		// write into array - for each vertex of the face the same value
		tangents[shape->eleBuf.at(i) * 3] = tan0.x;
		tangents[shape->eleBuf.at(i) * 3 + 1] = tan0.y;
		tangents[shape->eleBuf.at(i) * 3 + 2] = tan0.z;

		tangents[shape->eleBuf.at(i + 1) * 3] = tan1.x;
		tangents[shape->eleBuf.at(i + 1) * 3 + 1] = tan1.y;
		tangents[shape->eleBuf.at(i + 1) * 3 + 2] = tan1.z;

		tangents[shape->eleBuf.at(i + 2) * 3] = tan2.x;
		tangents[shape->eleBuf.at(i + 2) * 3 + 1] = tan2.y;
		tangents[shape->eleBuf.at(i + 2) * 3 + 1] = tan2.z;

		binormals[shape->eleBuf.at(i) * 3] = bin0.x;
		binormals[shape->eleBuf.at(i) * 3 + 1] = bin0.y;
		binormals[shape->eleBuf.at(i) * 3 + 2] = bin0.z;

		binormals[shape->eleBuf.at(i + 1) * 3] = bin1.x;
		binormals[shape->eleBuf.at(i + 1) * 3 + 1] = bin1.y;
		binormals[shape->eleBuf.at(i + 1) * 3 + 2] = bin1.z;

		binormals[shape->eleBuf.at(i + 2) * 3] = bin2.x;
		binormals[shape->eleBuf.at(i + 2) * 3 + 1] = bin2.y;
		binormals[shape->eleBuf.at(i + 2) * 3 + 1] = bin2.z;
	}
	// Copy the tangent and binormal to meshData
	for (unsigned int i = 0; i < shape->posBuf.size(); i++) {
		shape->tanBuf.push_back(tangents[i]);
		shape->binormBuf.push_back(binormals[i]);
	}
}


void GetTangent(
	glm::vec3 *posA, glm::vec3 *posB, glm::vec3 *posC,
	glm::vec2 *texA, glm::vec2 *texB, glm::vec2 *texC,
	glm::vec3 *nnA, glm::vec3 *nnB, glm::vec3 *nnC,
	glm::vec3 *tanA, glm::vec3 *tanB, glm::vec3 *tanC)
{

	if (!posA || !posB || !posC) return;
	if (!texA || !texB || !texC) return;
	if (!nnA || !nnB || !nnC) return;

	glm::vec3 v1 = *posA;
	glm::vec3 v2 = *posB;
	glm::vec3 v3 = *posC;

	glm::vec2 w1 = *texA;
	glm::vec2 w2 = *texB;
	glm::vec2 w3 = *texC;





	float x1 = v2.x - v1.x;
	float x2 = v3.x - v1.x;
	float y1 = v2.y - v1.y;
	float y2 = v3.y - v1.y;
	float z1 = v2.z - v1.z;
	float z2 = v3.z - v1.z;

	float s1 = w2.x - w1.x;
	float s2 = w3.x - w1.x;
	float t1 = w2.y - w1.y;
	float t2 = w3.y - w1.y;
	float inf = (s1 * t2 - s2 * t1);
	if (inf == 0)	inf = 0.0001;
	float r = 1.0F / inf;
	glm::vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
		(t2 * z1 - t1 * z2) * r);
	glm::vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
		(s1 * z2 - s2 * z1) * r);


	glm::vec3 erg;

	erg = sdir - (*nnA) * glm::dot(*nnA, sdir);
	erg =glm::normalize(erg);

	*tanA = erg;
	if (tanB)
	{
		erg = sdir - (*nnB) * glm::dot(*nnB, sdir);
		erg = glm::normalize(erg);
		*tanB = erg;
	}
	if (tanC)
	{
		erg = sdir - (*nnC) * glm::dot(*nnC, sdir);
		erg = glm::normalize(erg);
		*tanC = erg;
	}
}
