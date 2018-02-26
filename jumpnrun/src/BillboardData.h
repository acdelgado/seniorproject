#pragma once
#include <iostream>
#include <vector>

#include "glm/glm.hpp"

using namespace std;
using namespace glm;


class BillboardData {
public:
	BillboardData(string name, string texture, vector<glm::vec3> points, vector<glm::vec2> tcoords, long vertexCount);


	vector<glm::vec3> points;
	vector<glm::vec2> texcoords;
	string name;
	long vertexCount;
	string texture;
};