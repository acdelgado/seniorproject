#include "BillboardData.h"
#include "glm/glm.hpp"

using namespace std;
using namespace glm;


BillboardData::BillboardData(string n, string t, vector<glm::vec3> points_, vector<glm::vec2> tcoords, long vertexCount, short i) 
{
   name = n;
   id = i ;
   texture = t;
   points = points_;
   vertexCount = vertexCount;
   texcoords = tcoords;
}