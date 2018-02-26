#pragma once
#include <iostream>
#include <glad/glad.h>

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
#include <set>

using namespace std;
using namespace glm;

using namespace std;

class Billboard {
public:
	GLuint VAO;
	GLuint VBO, TexBuffer;

	shared_ptr<Program> *prog;

	glm::mat4 M;
	glm::vec3 position, speed;

	float z;
	float frame;

	void init(shared_ptr<Program> *program, float zposition);
	void draw(bool depthTest, float x, float y);
};
