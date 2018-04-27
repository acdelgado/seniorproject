#include "client.h"

client_ client;
char g_ip[100];
int g_port;
DWORD WINAPI start_client_thread(LPVOID args)
	{
	int *pport = (int*)(args);
	client.run(g_ip,g_port);
	return 0;
	}

void set_outgoing_data_packet(client_data_packet_  &data)
	{
	client.my_data = data;
	}
void get_incomming_data_packet(server_data_packet_ &data)
	{
	data = client.server_data_packet;
	}
void start_client(char *ip, int port)
	{
	DWORD threadId;
	int *pport = &port;
	strcpy(g_ip, ip);
	g_port = port;
	CreateThread(NULL, 0, start_client_thread, pport, 0, &threadId);
	int z;
	z = 0;

	}