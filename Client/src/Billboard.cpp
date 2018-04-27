#include "GLSL.h"
#include "Billboard.h"
#include "Program.h"

using namespace std;

void Billboard::init(shared_ptr<Program> *program, float zposition)
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
}

void Billboard::draw(bool depthTest, float x, float y) 
{
	if (!depthTest)
		glDisable(GL_DEPTH_TEST);
	glBindVertexArray(VAO);

	M = glm::translate(glm::mat4(1), glm::vec3(x, y, z));
	glUniformMatrix4fv((*prog)->getUniform("M"), 1, GL_FALSE, &M[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_DEPTH_TEST);
}