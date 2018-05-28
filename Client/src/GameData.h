#pragma once

#ifndef GAME_DATA
#define GAME_DATA

#define MAX_PLAYERS 30
#define MAX_OBJECTS 5

#include "BillboardFile.h"
#include "glm/glm.hpp"
#include "data_packet.h"

using namespace std;

class gamePlayer {
public:
	int id;
	glm::vec2 position;
	glm::vec2 impulse;
	bool isDead;

	gamePlayer() {
		id = 0;
		position = glm::vec2(0, 0);
		impulse = glm::vec2(0, 0);
		isDead = FALSE;
	};
};

class fallingObject {
public:
	int id;
	GLfloat data[12];
	float impulseY;
	float diffY;
	bool isFalling;
	string texture;

	fallingObject() {
		id = 0;
		isFalling = false;
		impulseY = 0;
		diffY = 0;
	}

	void readBillboardData(BillboardData bbd) {
		texture = bbd.texture;

		int verc = 0;
		data[verc++] = bbd.points[0][0], data[verc++] = bbd.points[0][1];
		data[verc++] = bbd.points[1][0], data[verc++] = bbd.points[1][1];
		data[verc++] = bbd.points[2][0], data[verc++] = bbd.points[2][1];
		data[verc++] = bbd.points[3][0], data[verc++] = bbd.points[3][1];
		data[verc++] = bbd.points[4][0], data[verc++] = bbd.points[4][1];
		data[verc++] = bbd.points[5][0], data[verc++] = bbd.points[5][1];
	}
};

class gameData {
public:
	short level;
	bool active;
	int numPlayers;
	glm::vec3 camera_pos;
	glm::vec3 camera_rot;
	gamePlayer host;
	gamePlayer players[MAX_PLAYERS];
	fallingObject objects[MAX_OBJECTS];

	gameData() {
		level = 2;
		active = false;
		numPlayers = 0;
		camera_pos = glm::vec3(0, 0, 0);
		host = gamePlayer();
	}

};

void copyToServerPacket(gameData *gd, server_data_packet_ *dp) {
	int size = sizeof(gameData);

	memcpy((char *)(dp->datafloat), (char *)gd, size);
}

void getFromServerPacket(gameData *gd, server_data_packet_ *dp) {
	int size = sizeof(gameData);

	memcpy((char *)gd, (char *)(dp->datafloat), size);
}

void copyClientsToGameData(gameData *gd, client_data_packet_ *dp) {
	int active = 0;
	bool hostStillActive = false;

	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (dp[i].dataint[0] != 0) {
			gd->players[active].id = dp[i].dataint[0];
			gd->players[active].position = glm::vec2(dp[i].datafloat[0], dp[i].datafloat[1]);
			gd->players[active].impulse = glm::vec2(dp[i].datafloat[2], dp[i].datafloat[3]);

			if (!gd->host.id) {
				gd->host = gd->players[0];
				hostStillActive = true;
				cout << gd->host.id << " is new host." << endl;
			}
			else if (dp[i].dataint[0] == gd->host.id) {
				hostStillActive = true;
				if (dp[i].dataint[1] == 1) {
					gd->active = true;
				}
			}
			active++;

			for (int j = 0; j < MAX_OBJECTS; j++) {
				if (dp[i].dataint[j + 2] != 0) {
					for (int k = 0; k < MAX_OBJECTS; k++) {
						if (gd->objects[k].id == dp[i].dataint[j + 1])
							gd->objects[k].isFalling = true;
					}
				}
			}
		}
	}

	gd->numPlayers = active;
	if (!hostStillActive && active > 0) {
		gd->host = gd->players[0];
		cout << "Host disconnected. New host is " << gd->host.id << endl;
	}

	for (int i = active; i < MAX_PLAYERS; i++) {
		gd->players[i].id = 0;
		gd->players[i].position.x = 0.0;
		gd->players[i].position.y = 0.0;
		gd->players[i].impulse.x = 0.0;
		gd->players[i].impulse.x = 0.0;
	}
}

#endif // !GAME_DATA
