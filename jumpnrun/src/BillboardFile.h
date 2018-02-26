#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "BillboardData.h"
#include "glm/glm.hpp"

using namespace std;

class BillboardFile {
public:

   BillboardFile(string filename);
   BillboardData getData(ifstream &input, string line);
   vector<BillboardData> getAll();

private:
   string filename;
};