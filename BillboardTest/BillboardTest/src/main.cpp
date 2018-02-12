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
#include "Billboard.h"
#include "controller.h"
#include <set>

using namespace std;
using namespace glm;




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

	std::shared_ptr<Billboard> bill;

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

		bill = make_shared<Billboard>();
		bill->init(&scoreprog, -10);
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/asteroid.obj");
		shape->resize();
		shape->calc_SxT();
		shape->init();

		shape2 = make_shared<Shape>();
		shape2->loadMesh(resourceDirectory + "/FA18.obj");
		shape2->resize();
		shape2->calc_SxT();
		shape2->init();

		lasershape = make_shared<Shape>();
		lasershape->loadMesh(resourceDirectory + "/sphere.obj");
		lasershape->resize();
		lasershape->init();

		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/moon.jpg";
		strcpy(filepath, str.c_str());
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
		//texture 2
		str = resourceDirectory + "/moon_normalmap.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		str = resourceDirectory + "/stars.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture3);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture3);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/FA18.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture4);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture4);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		
		str = resourceDirectory + "/explosion.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureExplosion);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureExplosion);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/mariobitmap.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureScore);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureScore);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		GLuint Tex3Location = glGetUniformLocation(prog2->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex4Location = glGetUniformLocation(prog2->pid, "tex2");
		GLuint Tex5Location = glGetUniformLocation(shipprog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex6Location = glGetUniformLocation(shipprog->pid, "tex2");
		GLuint TexplosionLocation = glGetUniformLocation(billprog->pid, "tex");
		GLuint TexScoreLocation = glGetUniformLocation(scoreprog->pid, "tex");

		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		glUseProgram(prog2->pid);
		glUniform1i(Tex3Location, 0);
		glUniform1i(Tex4Location, 1);

		glUseProgram(shipprog->pid);
		glUniform1i(Tex5Location, 0);
		glUniform1i(Tex6Location, 1);

		glUseProgram(billprog->pid);
		glUniform1i(TexplosionLocation, 0);

		glUseProgram(scoreprog->pid);
		glUniform1i(TexScoreLocation, 0);
	}
	void render()
	{
		
		if (gamepad->IsConnected())
		{
		if (gamepad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_Y)
			mycam.w = 1;
		else
			mycam.w = 0;
		if (gamepad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_A)
			{
			
			mycam.s = 1;
			}
		else
			mycam.s = 0;

		}
		SHORT lx = gamepad->GetState().Gamepad.sThumbLX;
		SHORT ly = gamepad->GetState().Gamepad.sThumbLY;

		if (abs(ly) > 3000)
			{
			float angle_x = (float)ly / 32000.0;
			angle_x *= 0.05;
			mycam.rot.x -= angle_x;
			}
		if (abs(lx) > 3000)
			{
			float angle_y = (float)lx / 32000.0;
			angle_y *= 0.05;
			mycam.rot.y -= angle_y;
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
		glBindTexture(GL_TEXTURE_2D, TextureScore);

		bill->draw(depth,scorecoord.x,scorecoord.y);

		scoreprog->unbind();

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
