//*****************************************************************************************************
//
//								MULTIPLAYER (at the moment: 2)
//
//										SERVER
//
//				Usage: 
//
//				0. Include the "cserver.h"
//
//				1. Starting the server (InitDevice() pr OnCreate()) with
//
//						start_server(27015);	//default port is 27015
//
//				2. Have an outgoing (to the cient) data buffer anytime
//
//						server_data_packet_ temp_outgoing;
//							//fill it with data, tehre is integer and float data available
//						set_outgoing_data_packet(temp_outgoing);
//
//				3. Have an incomming (from the cient) data buffer anytime
//
//						client_data_packet_ temp_incomming;
//							//fill it with data, tehre is integer and float data available
//						get_incomming_data_packet(temp_incomming);
//
//				4. Bot data buffers have a fixed size with floats and ints, see "data_packet.h". Change if necessary.
//
//*****************************************************************************************************



#pragma once
#include "data_packet.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 4096
#define DEFAULT_PORT "27015"

class server_
	{
	private:
		WSADATA wsaData;
		int iResult;

		SOCKET ListenSocket = INVALID_SOCKET;
		SOCKET ClientSocket = INVALID_SOCKET;

		struct addrinfo *result = NULL;
		struct addrinfo hints;

		int number_of_clients = 0;

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
		int resolve_server(int port)
			{
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			hints.ai_flags = AI_PASSIVE;

			char cport[100];
			sprintf(cport, "%d", port);
			// Resolve the server address and port
			iResult = getaddrinfo(NULL, cport, &hints, &result);
			if (iResult != 0) {
				printf("getaddrinfo failed with error: %d\n", iResult);
				WSACleanup();
				return 1;
				}
			return 0;
			}
		//-----------------------------------------------------
		int connection_socket()
			{
			// Create a SOCKET for connecting to server
			ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (ListenSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				freeaddrinfo(result);
				WSACleanup();
				return 1;
				}

			// Setup the TCP listening socket
			int i = (int)result->ai_addrlen;
			iResult = ::bind(ListenSocket, result->ai_addr, i);
			if (iResult == SOCKET_ERROR) {
				printf("bind failed with error: %d\n", WSAGetLastError());
				freeaddrinfo(result);
				closesocket(ListenSocket);
				WSACleanup();
				return 1;
				}

			

			freeaddrinfo(result);
			return 0;
			}
		//-----------------------------------------------------
		int listen_to_clients()
			{
			iResult = listen(ListenSocket, SOMAXCONN);
			if (iResult == SOCKET_ERROR) {
				printf("listen failed with error: %d\n", WSAGetLastError());
				closesocket(ListenSocket);
				WSACleanup();
				return 1;
				}

			// Accept a client socket
			ClientSocket = accept(ListenSocket, NULL, NULL);
			if (ClientSocket == INVALID_SOCKET) {
				printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(ListenSocket);
				WSACleanup();
				return 1;
				}
			number_of_clients++;
			// No longer need server socket
			closesocket(ListenSocket);
			return 0;
			}
		//-----------------------------------------------------
		int shutdown_and_cleanup()
			{
			// shutdown the connection since we're done
			iResult = shutdown(ClientSocket, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
				}

			// cleanup
			closesocket(ClientSocket);
			WSACleanup();
			return 0;
			}

		int send_data(char *data)
			{
			iResult = send(ClientSocket, data, sizeof(data), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
				}
			return 0;
			}
		int receive_data(char *data, int maxlen)
			{
			int iResult = recv(ClientSocket, data, maxlen, MSG_WAITALL);
			/*if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
			else if (iResult == 0)
			printf("Connection closed\n");
			else
			printf("recv failed with error: %d\n", WSAGetLastError());*/
			return iResult;
			}
		bool init_connection(int port)
			{
			if (init_winsocks())
				return false;
			
			if (resolve_server(port))
					return false;

			if (connection_socket())
					return false;
			if (listen_to_clients())
				return false;
			return true;
			}
		//----------------------------------------------------------------------------------------------------------------
		public:
			void run(int port)
			{
			if (!init_connection(port))
				return;
			bool running = true;
			client_data_packet_ temp;
			server_data_packet_ temp_server_data;
			while (running)
				{
				for (int ii = 0; ii < number_of_clients; ii++)
					{
					int res = receive_data(temp.get_address(), temp.get_size());
					if (res <= 0)break;//error or connection closed
					client_data[ii] = temp;
					}
				temp_server_data = data_packet;
				for (int ii = 0; ii < number_of_clients; ii++)
					{
					int res = send(ClientSocket, temp_server_data.get_address(), temp_server_data.get_size(),0);
					if (res == SOCKET_ERROR)break;//error or connection closed
					}			
				}

			close_connection();			
			}
	
		server_data_packet_ data_packet;
		client_data_packet_ client_data[20];//20 possible clients
		//std::thread
	
		bool close_connection()
			{
			if (shutdown_and_cleanup())
				return false;	
			return true;
			}
		server_()
			{

			}
	};

	void set_outgoing_data_packet(server_data_packet_ &data);
	void get_incomming_data_packet(client_data_packet_ &data);
	void start_server(int port = 27015);