#include "BillboardData.h"
#include "glm/glm.hpp"

using namespace std;
using namespace glm;


BillboardData::BillboardData(string name, string texture, vector<glm::vec3> points, long vertexCount) {
   this->name = name;
   this->texture = texture;
   this->points = points;
   this->vertexCount = vertexCount;
}