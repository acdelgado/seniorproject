/* Lab 6 base code - transforms using local matrix functions
to be written by students -
based on lab 5 by CPE 471 Cal Poly Z. Wood + S. Sueda
& Ian Dunn, Christian Eckhardt
Modified heavily by Anthony Delgado for the
implementation of a 3D version of the 1979 arcade classic, "Asteroids"
*/
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <glad/glad.h>
#include <set>
#include <unordered_map>

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
#include "client.h"
#include "cserver.h"
#include "Stopwatch.h"
#include "GameData.h"


#define GRAVITY 35

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
private:
	StopWatchMicro_ timer = StopWatchMicro_();
public:
	vec2 pos, impulse;
	int lifes;
	int animate;
	long double jump_start;
	bool moving;
	bool jumping;
	bool isDead;

	character()
	{
		pos = vec2(-10, 1);
		impulse = vec2(0, 0);
		isDead = FALSE;
		moving = FALSE;
		jumping = FALSE;
		animate = 1;
		timer.start();
	}
	void process(float ftime)
	{
		if (impulse.y == 0 && !moving && impulse.x != 0) {
			if (impulse.x > 0) {
				impulse.x = max(impulse.x - 30.0 * ftime, 0.0);
			}
			else {
				impulse.x = min(impulse.x + 30.0 * ftime, 0.0);
			}
		}

		pos += impulse * ftime;
		impulse.y -= GRAVITY * ftime;
		if (pos.y < -20) //death fall y-value
		{
			this->reset();
		}

		//animate
		if (impulse.x < 0)
			animate = -1;
		if (impulse.x > 0)
			animate = 1;

	}

	void start_jump()
	{
		timer.start();
		jump_start = timer.elapse_milli();
	}

	bool end_jump()
	{
		return timer.elapse_milli() > jump_start + (300);
	}

	void reset()
	{
		pos.x = -10;
		pos.y = 1;
		impulse.x = 0;
		impulse.y = 0;
		isDead = FALSE;
	}

	void dead()
	{
		impulse.x = 0;
		impulse.y = 12;
		isDead = TRUE;
	}
};

character player;
unordered_map<int, character> others;

class Billboard {
public:
	string name;
	GLuint VAO;
	GLuint VBO, TexBuffer;
	GLuint Texture, Texture2;
	shared_ptr<Program> *prog;
	GLfloat *data;

	glm::mat4 M;
	glm::vec3 position, speed;

	float z;
	float frame;
	float drawY;
	float diffY;
	bool falling;
	bool triggered;
	float impulseY;
	short id;

	void init(BillboardData *bbd, string resourceDir)
	{
		name = bbd->name;
		z = bbd->points[0][2];
		triggered = false;
		falling = false;
		drawY = 0;
		diffY = 0;
		impulseY = 0;
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

		//set id
		id = bbd->id;

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &TexBuffer);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, TexBuffer);

		GLfloat *cube_tex = new GLfloat[12];
		int texc = 0;
		cube_tex[texc++] = 0, cube_tex[texc++] = 0;
		cube_tex[texc++] = 1, cube_tex[texc++] = 0;
		cube_tex[texc++] = 1, cube_tex[texc++] = 1;
		cube_tex[texc++] = 0, cube_tex[texc++] = 0;
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
		//cout << bbd->texture.c_str() << endl;
		strcpy(filepath, (resourceDir + bbd->texture).c_str());
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


	}

	void bindToProg(shared_ptr<Program> *program)
	{
		GLuint TexLocation = glGetUniformLocation((*program)->pid, "tex");

		glUseProgram((*program)->pid);
		glUniform1i(TexLocation, 0);
	}

	void init(shared_ptr<Program> *program, float zposition, string imageFile)
	{
		prog = program;
		z = zposition;
		falling = false;
		//generate the VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		drawY = 0;

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VBO);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		data = new GLfloat[18];
		int verc = 0;
		data[verc++] = -1.0, data[verc++] = -1.0, data[verc++] = 0.0;
		data[verc++] = 1.0, data[verc++] = -1.0, data[verc++] = 0.0;
		data[verc++] = -1.0, data[verc++] = 1.0, data[verc++] = 0.0;
		data[verc++] = 1.0, data[verc++] = -1.0, data[verc++] = 0.0;
		data[verc++] = 1.0, data[verc++] = 1.0, data[verc++] = 0.0;
		data[verc++] = -1.0, data[verc++] = 1.0, data[verc++] = 0.0;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), data, GL_STATIC_DRAW);
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

	void addTexture(shared_ptr<Program> *program, string texName)
	{
		int width, height, channels;
		char filepath[1000];
		strcpy(filepath, texName.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		GLuint TexLocation = glGetUniformLocation((*program)->pid, "tex2");

		glUseProgram((*program)->pid);
		glUniform1i(TexLocation, 1);
	}

	void draw(shared_ptr<Program> *program, bool depthTest, float x, float y, float epsilon)
	{
		if (!depthTest)
			glDisable(GL_DEPTH_TEST);
		glBindVertexArray(VAO);

		M = glm::translate(glm::mat4(1), glm::vec3(x, y, z - epsilon - 50));
		glUniformMatrix4fv((*program)->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
	}

	void updatePos(double ftime)
	{
		if (falling)
		{
			impulseY += GRAVITY * ftime / 50000;
			diffY += impulseY;
			drawY += diffY;
			int verc = 0;
			for (int i = 0; i < 6; i++)
				data[i * 2 + 1] -= diffY;
		}
	}
};

vector<Billboard> bill;
Billboard sprite, sprite_w, sprite_r, sprite_j;

class Application : public EventCallbacks
{
public:
	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> scoreprog;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape;
	shared_ptr<Shape> shape2, lasershape, sun;

	//camera
	camera mycam;

	//texture for sim
	GLuint Texture;
	GLuint Texture2, Texture3, Texture4, Texture5, TextureExplosion;

	// Relevant game data
	gameData game;
	bool hostStarting;
	int id;

	int t = 0;
	vec2 offset = vec2(0, 0);

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
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
		{
			mycam.e = 1;
		}
		if (key == GLFW_KEY_E && action == GLFW_RELEASE)
		{
			mycam.e = 0;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		{
			mycam.q = 1;
		}
		if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
		{
			mycam.q = 0;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		int winX, winY;

		if (action == GLFW_PRESS)
		{
			// FOR DEBUGGING AND TESTING PURPOSES
			glfwGetCursorPos(window, &posX, &posY);
			glfwGetWindowSize(window, &winX, &winY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
			float relX = posX - (winX / 2);
			float relY = posY - (winY / 2);
			player.pos.x = mycam.pos.x + relX / 20;
			player.pos.y = -mycam.pos.y - relY / 20;
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

		scoreprog = make_shared<Program>();
		scoreprog->setVerbose(true);
		scoreprog->setShaderNames(resourceDirectory + "scorevert.glsl", resourceDirectory + "scorefrag.glsl");
		if (!scoreprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		scoreprog->init();
		scoreprog->addUniform("P");
		scoreprog->addUniform("V");
		scoreprog->addUniform("M");
		scoreprog->addUniform("offset");
		scoreprog->addUniform("animate");
		scoreprog->addUniform("diffColor");
		scoreprog->addUniform("wave");
		scoreprog->addUniform("flip");
		scoreprog->addAttribute("vertPos");
		scoreprog->addAttribute("vertTex");


		srand(time(NULL));
		id = rand() + 1;
		hostStarting = false;
	}

	void initGeom(const std::string& resourceDirectory)
	{

		sprite.init(&scoreprog, 0, resourceDirectory + "smol_idle.png");
		sprite_w.init(&scoreprog, 0, resourceDirectory + "sprite_walk.png");
		sprite_r.init(&scoreprog, 0, resourceDirectory + "smol_run.png");
		sprite_j.init(&scoreprog, 0, resourceDirectory + "smol_jump.png");
		BillboardFile test = BillboardFile(resourceDirectory + "level3.grl");
		vector<BillboardData> things = test.getAll();
		bill.resize(things.size());
		for (int ii = 0; ii < things.size(); ii++)
		{
			bill[ii].init(&things[ii], resourceDirectory);
			bill[ii].bindToProg(&scoreprog);
			if (bill[ii].name == "water")
			{
				bill[ii].addTexture(&scoreprog, resourceDirectory + "displacement.png");
			}
		}
	}
	void render()
	{

		double ftime = get_last_elapsed_time();

		for (int ii = 0; ii < bill.size(); ii++)
		{
			bill[ii].updatePos(ftime);
			if (bill[ii].name == "rock" && bill[ii].falling) {
				int i = 0;
			}

			if (!player.isDead && sprite.z == bill[ii].z) {
				float minX = bill[ii].data[0];
				float maxX = bill[ii].data[4];
				float minY = bill[ii].data[1];
				float maxY = bill[ii].data[5];

				if (inSquare(vec2(player.pos.x - 1, player.pos.y), minX, maxX, minY, maxY))
				{
					if (player.impulse.x < -6)
						player.impulse.x = -player.impulse.x / 3;
					else
						player.impulse.x = 0;
					player.pos.x = maxX + 1;
				}

				if (inSquare(vec2(player.pos.x + 1, player.pos.y), minX, maxX, minY, maxY))
				{
					if (player.impulse.x > 6)
						player.impulse.x = -player.impulse.x / 3;
					else
						player.impulse.x = 0;
					player.pos.x = minX - 1;
				}

				if (inSquare(vec2(player.pos.x, player.pos.y - 0.8), minX, maxX, minY, maxY))
				{
					player.pos.y = maxY + 0.8;
					player.impulse.y = 0;
					if (bill[ii].name == "upspike") {
						player.dead();
					}
					if (bill[ii].name == "rock")
					{
						bill[ii].triggered = true;
					}
				}

				if (inSquare(vec2(player.pos.x, player.pos.y + 1), minX, maxX, minY, maxY))
				{
					player.pos.y = minY - 1;
					player.impulse.y = -1;
					if (bill[ii].name == "downspike" || (bill[ii].name == "rock" && bill[ii].falling)) {
						player.dead();
					}
				}
			}
		}

		if (gamepad->IsConnected())
		{
			if (game.active && !player.isDead) {
				if (gamepad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_A)
				{
					if (!player.jumping && player.impulse.y == 0.0) {
						player.jumping = true;
						player.start_jump();
						player.impulse.y = 10.0;
					}
					else if (player.jumping && !player.end_jump()) {
						player.impulse.y = min(player.impulse.y + 1.5, 12.5);
					}
				}
				else if (player.impulse.y == 0.0) {
					player.jumping = false;
				}


				SHORT lx = gamepad->GetState().Gamepad.sThumbLX;
				SHORT ly = gamepad->GetState().Gamepad.sThumbLY;

				if (lx > 3000 || lx < (-3000))
				{
					double max_speed = 6.0;
					if (gamepad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_X) {
						max_speed = 10.0;
					}
					player.moving = true;
					float nx = (float)lx / 32000.0;
					if (nx > 0) {
						player.impulse.x = min(player.impulse.x + 0.5 * nx, max_speed);
					}
					else {
						player.impulse.x = max(player.impulse.x + 0.5 * nx, -max_speed);
					}

				}
				else
					player.moving = false;
			}
			else if (gamepad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_A &&
				game.host.id == id) {
				hostStarting = true;
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
		glm::mat4 M, V, R, Sc;
		static float angle = 0;
		angle += 0.01;
		static vec3 eCord(0, 0, 0);

		//VIEW MATRIX
		V = mycam.process();

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scoreprog->bind();

		bool depth = true;

		//S
		M = glm::translate(mat4(1), glm::vec3(0, 0, -3));
		glUniformMatrix4fv(scoreprog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(scoreprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(scoreprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);

		t++;
		if (t % 5 == 0) {
			offset.x += 1 / 3.0f;
			if (offset.x == 1) {
				offset.x = 0;
				offset.y += 1 / 3.0f;
				if (offset.y == 1) offset.y = 0;
			}
			if (offset.x == 1 / 3.0f && offset.y == 0)
			{
				offset.x = 0;
				offset.y = 1 / 3.0f;
			}

		}
		glUniform2fv(scoreprog->getUniform("offset"), 1, &offset.x);
		glUniform1f(scoreprog->getUniform("animate"), 0);
		glUniform1f(scoreprog->getUniform("diffColor"), 1);
		glActiveTexture(GL_TEXTURE0);


		float wave = glfwGetTime();
		float flip = 0;
		for (int ii = 0; ii < bill.size(); ii++) {
			if (bill[ii].name == "water") {
				glDisable(GL_DEPTH_TEST);
				glUniform1f(scoreprog->getUniform("wave"), wave);
				glUniform1f(scoreprog->getUniform("flip"), flip);
				flip = 1;

				bill[ii].draw(&scoreprog, depth, 0, -mycam.pos.y, 0);
				glEnable(GL_DEPTH_TEST);
			}
			else {
				glUniform1f(scoreprog->getUniform("wave"), 0);
				bill[ii].draw(&scoreprog, depth, 0, -bill[ii].drawY, 0);
			}
		}


		glUniform1f(scoreprog->getUniform("wave"), 0);
		int inc = 1;
		glUniform1f(scoreprog->getUniform("diffColor"), 0);
		glDisable(GL_DEPTH_TEST);
		for (auto it = others.begin(); it != others.end(); ++it) {
			int ep = it->first;
			character temp = it->second;
			temp.process(ftime);
			glUniform1f(scoreprog->getUniform("animate"), temp.animate);
			display_sprite(temp, depth, 0.00001f * ep);
			inc++;
		}
		glEnable(GL_DEPTH_TEST);

		player.process(ftime);

		glUniform1f(scoreprog->getUniform("animate"), player.animate);
		glUniform1f(scoreprog->getUniform("diffColor"), 1);
		if (player.isDead)
			display_sprite(player, depth, -0.5);
		else
			display_sprite(player, depth, 0);
		scoreprog->unbind();

	}

	void display_sprite(character & c, bool depth, float epsilon)
	{
		if (c.impulse.y > 0)
			sprite_j.draw(&scoreprog, depth, c.pos.x, c.pos.y, epsilon);
		if (c.impulse.x == 0)
			sprite.draw(&scoreprog, depth, c.pos.x, c.pos.y, epsilon);
		else if (abs(c.impulse.x) <= 6)
			sprite_w.draw(&scoreprog, depth, c.pos.x, c.pos.y, epsilon);
		else {
			sprite_r.draw(&scoreprog, depth, c.pos.x, c.pos.y, epsilon);
		}
	}
	bool inSquare(vec2 p, float minX, float maxX, float minY, float maxY)
	{
		return p.x > minX && p.x < maxX && p.y > minY && p.y < maxY;
	}

};
//*********************************************************************************************************
int main(int argc, char **argv) {
	StopWatchMicro_ sw;
	sw.start();

	// Where the resources are loaded from
	std::string resourceDir = "../resources/";

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

	/*start_client("129.65.221.104", 27015);*/
	start_client("127.0.0.1", 27015);

	client_data_packet_ cp;
	server_data_packet_ incoming;
	cp.dataint[0] = application->id;

	long double last_send = sw.elapse_milli();

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		if (sw.elapse_milli() > last_send + 5) {
			cp.datafloat[0] = player.pos.x;
			cp.datafloat[1] = player.pos.y;
			cp.datafloat[2] = player.impulse.x;
			cp.datafloat[3] = player.impulse.y;

			if (application->hostStarting) {
				cp.dataint[1] = 1;
			}

			for (int i = 0; i < bill.size(); i++)
				cp.dataint[2 + i] = 0;

			int rockIndex = 2;
			for (int i = 0; i < bill.size(); i++) {
				if (bill[i].triggered) {
					bill[i].triggered = false;
					cp.dataint[rockIndex++] = bill[i].id;
				}
			}

			set_outgoing_data_packet(cp);

			last_send = sw.elapse_milli();

			get_incomming_data_packet(incoming);
			others.clear();

			getFromServerPacket(&application->game, &incoming);

			for (int i = 0; i < MAX_PLAYERS; i++) {
				if (application->game.players[i].id != 0) {
					int tempid = application->game.players[i].id;
					if (tempid == application->id)
						continue;
					others[tempid].pos = application->game.players[i].position;
					others[tempid].impulse = application->game.players[i].impulse;
				}
			}

			for (int i = 0; i < MAX_OBJECTS; i++) {
				fallingObject temp = application->game.objects[i];
				for (int j = 0; j < bill.size(); j++) {
					if (temp.id == bill[j].id) {
						bill[j].falling = temp.isFalling;
						for (int verc = 0; verc < 6; verc++)
							bill[j].data[verc * 2 + 1] = temp.data[verc * 2 + 1] - temp.totDiff;
						//std::memcpy(bill[j].data, temp.data, sizeof(GLfloat) * 12);
						bill[j].impulseY = temp.impulseY;
						bill[j].diffY = temp.diffY;
						bill[j].drawY = temp.drawY;
					}
				}
			}
			application->game.active = true;
			//application->mycam.pos.y = application->game.camera_pos.y;
		}

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



		glm::vec3 vertex0 = glm::vec3(shape->posBuf.at(shape->eleBuf.at(i) * 3), shape->posBuf.at(shape->eleBuf.at(i) * 3 + 1), shape->posBuf.at(shape->eleBuf.at(i) * 3 + 2));
		glm::vec3 vertex1 = glm::vec3(shape->posBuf.at(shape->eleBuf.at(i + 1) * 3), shape->posBuf.at(shape->eleBuf.at(i + 1) * 3 + 1), shape->posBuf.at(shape->eleBuf.at(i + 1) * 3 + 2));
		glm::vec3 vertex2 = glm::vec3(shape->posBuf.at(shape->eleBuf.at(i + 2) * 3), shape->posBuf.at(shape->eleBuf.at(i + 2) * 3 + 1), shape->posBuf.at(shape->eleBuf.at(i + 2) * 3 + 2));

		glm::vec3 normal0 = glm::vec3(shape->norBuf.at(shape->eleBuf.at(i) * 3), shape->norBuf.at(shape->eleBuf.at(i) * 3 + 1), shape->norBuf.at(shape->eleBuf.at(i) * 3 + 2));
		glm::vec3 normal1 = glm::vec3(shape->norBuf.at(shape->eleBuf.at(i + 1) * 3), shape->norBuf.at(shape->eleBuf.at(i + 1) * 3 + 1), shape->norBuf.at(shape->eleBuf.at(i + 1) * 3 + 2));
		glm::vec3 normal2 = glm::vec3(shape->norBuf.at(shape->eleBuf.at(i + 2) * 3), shape->norBuf.at(shape->eleBuf.at(i + 2) * 3 + 1), shape->norBuf.at(shape->eleBuf.at(i + 2) * 3 + 2));

		glm::vec2 tex0 = glm::vec2(shape->texBuf.at(shape->eleBuf.at(i) * 2), shape->texBuf.at(shape->eleBuf.at(i) * 2 + 1));
		glm::vec2 tex1 = glm::vec2(shape->texBuf.at(shape->eleBuf.at(i + 1) * 2), shape->texBuf.at(shape->eleBuf.at(i + 1) * 2 + 1));
		glm::vec2 tex2 = glm::vec2(shape->texBuf.at(shape->eleBuf.at(i + 2) * 2), shape->texBuf.at(shape->eleBuf.at(i + 2) * 2 + 1));

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
	erg = glm::normalize(erg);

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