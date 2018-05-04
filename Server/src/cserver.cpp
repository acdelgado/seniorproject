#include "cserver.h"

server_ server;
int s_port;
DWORD WINAPI start_server_thread(LPVOID args)
	{
	int *pport = (int*)(args);
	server.run(s_port);
	return 0;
	}
void set_outgoing_data_packet(server_data_packet_ &data)
	{
	server.data_packet = data;
	}
void get_incoming_data_packet(client_data_packet_ &data)
	{
	data = server.client_data[0];
	}

void get_all_incoming_data(client_data_packet_ **data) {
	*data = server.client_data;
}

void start_server(int port)
	{
	DWORD threadId;
	int *pport = &port;
	s_port = port;
	CreateThread(NULL, 0, start_server_thread, pport, 0, &threadId);
	int z;
	z = 0;

	}