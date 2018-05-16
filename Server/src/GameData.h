#pragma once

#ifndef GAME_DATA
#define GAME_DATA

#define MAX_PLAYERS 50

#include "glm/glm.hpp"
#include "data_packet.h"

using namespace std;

class gamePlayer {
public:

	gamePlayer() {
		id = -1;
		position = glm::vec2(0, 0);
		impulse = glm::vec2(0, 0);
	};

	int id;
	glm::vec2 position;
	glm::vec2 impulse;
};

class gameData {
public:
	int level;
	int numPlayers;
	glm::vec3 camera_pos;
	glm::vec3 camera_rot;
	gamePlayer host;
	gamePlayer players[MAX_PLAYERS];

	gameData() {
		level = 1;
		numPlayers = 0;
		camera_pos = glm::vec3(0, 0, 0);
		camera_rot = glm::vec3(0, 0, 0);
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

	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (dp[i].dataint[0] != 0) {
			gd->players[active].id = dp[i].dataint[0];
			gd->players[active].position = glm::vec2(dp[i].datafloat[0], dp[i].datafloat[1]);
			gd->players[active].impulse = glm::vec2(dp[i].datafloat[2], dp[i].datafloat[3]);
			active++;
		}
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