#include "BillboardFile.h"
#include "BillboardData.h"

vector<string> split(const string &s) {
	stringstream ss(s);
	string item;
	vector<string> tokens;
	while (getline(ss, item, ' ')) {
		tokens.push_back(item);
	}
	return tokens;
}


BillboardFile::BillboardFile(string filename) {
   this->filename = filename;
}

BillboardData BillboardFile::getData(ifstream &input, string line) {
	string name = "none";
	string texture;
	vector<string> tokens;
	long vertexCount, parsedVecs = 0;
	vector<glm::vec3> points;
	vector<glm::vec2> texcoords;
	while (input) {
		if ((line[0] == '/' && line[1] == '/') || line.length() == 0) {
			getline(input, line);
			continue;
		}
		tokens = split(line);
		if (tokens[0] == "#") {
			if (tokens[1] == "name:" && tokens.size() > 2) {
				name = tokens[2];
			}
			else if (tokens[1] == "properties:") {
				;
			}
			else if (tokens[1] == "vertexcount:") {
				vertexCount = std::stol(tokens[2], nullptr);
			}
			else if (tokens[1] == "texture:") {
				if(tokens.size() > 2)
					texture = tokens[2];
				// in case there were spaces in the texture file path
				for (int i = 3; i < tokens.size(); i++) {
					texture += " " + tokens[i];
				}
			}
		}
		else {
			points.push_back(glm::vec3(std::stof(tokens[0]), 
				                       std::stof(tokens[1]), 
				                       std::stof(tokens[2])));
			texcoords.push_back(glm::vec2(std::stof(tokens[6]),
				std::stof(tokens[7])));
			parsedVecs++;
			if (parsedVecs == vertexCount) {
				break;
			}
		}
		getline(input, line);
	}

	return BillboardData(name, texture, points, texcoords, vertexCount);
}

vector<BillboardData> BillboardFile::getAll() {
   string line;
   vector<BillboardData> boards;

   ifstream input(filename);

   if (!input) {
	   std::cerr << "Unable to open file " << filename << std::endl;
   }

   getline(input, line);


   while (input) {
	   while ((line[0] == '/' && line[1] == '/') || line.length() == 0) {
		   getline(input, line);
	   }
	   if (line[0] == '#') {
		   boards.push_back(getData(input, line));
	   }
	   getline(input, line);
   }

   return boards;
}