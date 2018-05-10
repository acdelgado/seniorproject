//*****************************************************************************************************
//
//								MULTIPLAYER (at the moment: 2)
//
//										CLIENT
//
//				Usage: 
//
//				0. Include the "client.h"
//
//				1. Starting the client (InitDevice() pr OnCreate()) with
//
//						start_client("127.0.0.1",27015);	//IP address of the server, then default port is 27015
//
//				2. Have an outgoing (to the server) data buffer anytime
//
//						client_data_packet_ temp_outgoing;
//							//fill it with data, tehre is integer and float data available
//						set_outgoing_data_packet(temp_outgoing);
//
//				3. Have an incomming (from the server) data buffer anytime
//
//						server_data_packet_ temp_incomming;
//							//fill it with data, tehre is integer and float data available
//						get_incomming_data_packet(temp_incomming);
//
//				4. Bot data buffers have a fixed size with floats and ints, see "data_packet.h". Change if necessary.
//
//*****************************************************************************************************


#pragma once

#include "data_packet.h"
#include "Stopwatch.h"
#include <iostream>



// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 4096
#define DEFAULT_PORT "27015"

using namespace std;


class client_
	{
	private:
		WSADATA wsaData;
		SOCKET ConnectSocket = INVALID_SOCKET;
		struct addrinfo *result = NULL,
			*ptr = NULL,
			hints;



		int init_winsocks()
			{
			int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != 0) {
				printf("WSAStartup failed with error: %d\n", iResult);
				return 1;
				}
			return 0;
			}
		//-----------------------------------------------------
		int resolve_server_ip(char *ip,int port)
			{
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			char cport[100];
			sprintf(cport, "%d", port);
			// Resolve the server address and port
			int iResult = getaddrinfo(ip, cport, &hints, &result);
			if (iResult != 0) {
				printf("getaddrinfo failed with error: %d\n", iResult);
				WSACleanup();
				return 1;
				}
			return 0;
			}
		//-----------------------------------------------------
		int connect_to_server()
			{
			for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

				// Create a SOCKET for connecting to server
				ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
					ptr->ai_protocol);
				if (ConnectSocket == INVALID_SOCKET) {
					printf("socket failed with error: %ld\n", WSAGetLastError());
					WSACleanup();
					return 1;
					}

				// Connect to server.
				int iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
				if (iResult == SOCKET_ERROR) {
					closesocket(ConnectSocket);
					ConnectSocket = INVALID_SOCKET;
					continue;
					}
				break;
				}
			freeaddrinfo(result);
			if (ConnectSocket == INVALID_SOCKET) {
				printf("Unable to connect to server!\n");
				WSACleanup();
				return 1;
				}
			return 0;
			}
		//-----------------------------------------------------
		int shutdown_and_cleanup()
			{
			// shutdown the connection since no more data will be sent
			int iResult = shutdown(ConnectSocket, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
				}
			closesocket(ConnectSocket);
			WSACleanup();
			return 0;
			}

		int send_data(char *data)
			{
			int iResult = send(ConnectSocket, data, (int)strlen(data), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
				}
			return 0;
			}
		int receive_data(char *data,int maxlen)
			{
			StopWatchMicro_ sw;
			sw.start();
			//cout << "before recv: " << sw.elapse_micro() << endl;
			int iResult = recv(ConnectSocket, data, maxlen, MSG_WAITALL);
			//cout << "after recv: " << sw.elapse_micro() << endl;
			/*if (iResult > 0)
				printf("Bytes received: %d\n", iResult);
			else if (iResult == 0)
				printf("Connection closed\n");
			else
				printf("recv failed with error: %d\n", WSAGetLastError());*/
			return iResult;
			}
		bool init_connection(char *ip,int port)
			{
			if (init_winsocks())
				return false;
			if (resolve_server_ip(ip,port))
				return false;
			if (connect_to_server())
				return false;

			return true;
			}
			public:
		bool run(char *ip, int port)
			{
			if (!init_connection(ip, port))
				return false;
			bool running = true;
			client_data_packet_ temp;
			server_data_packet_ temp_server_data;
			StopWatchMicro_ sw;
			sw.start();

			while (running)
				{
				temp = my_data;
				//cout << "before send: " << sw.elapse_micro() << endl;
				int res = send(ConnectSocket, temp.get_address(), temp.get_size(), 0);
				//cout << "after send: " << sw.elapse_micro() << endl;

				if (res <= 0)break;//error or connection closed

				res = receive_data(temp_server_data.get_address(), temp_server_data.get_size());
				if (res <= 0)break;//error or connection closed
				server_data_packet = temp_server_data;
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				}

			return shutdown_and_cleanup();
			}

		server_data_packet_ server_data_packet;
		client_data_packet_ my_data;
	
		client_()
			{
			
			}
	};


	void set_outgoing_data_packet(client_data_packet_  &data);
	void get_incomming_data_packet(server_data_packet_ &data);
	void start_client(char *ip, int port = 27015);