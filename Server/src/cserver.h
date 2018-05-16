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
#include "Stopwatch.h"
#include <iostream>
#include <vector>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")


#define MAX_CLIENTS 50

#define DEFAULT_BUFLEN 4096
#define DEFAULT_PORT "27015"

using namespace std;


struct _clients_b {
	bool connected;
	SOCKET ss;
	sockaddr_in address;
};

class server_
	{
	private:
		WSADATA wsaData;
		int iResult;

		SOCKET ListenSocket = INVALID_SOCKET;
		SOCKET ClientSocket = INVALID_SOCKET;

		bool running = true;
		struct addrinfo *result = NULL;
		struct addrinfo hints;

		sockaddr_in server;
		int len, number_of_clients = 0; 


		_clients_b clients[MAX_CLIENTS];

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
			int j = 1;

			// Create a SOCKET for connecting to server
			ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (ListenSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				freeaddrinfo(result);
				WSACleanup();
				return 1;
				}

			setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&j, sizeof(j));

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


			iResult = listen(ListenSocket, MAX_CLIENTS);
			if (iResult == SOCKET_ERROR) {
				printf("listen failed with error: %d\n", WSAGetLastError());
				closesocket(ListenSocket);
				WSACleanup();
				return 1;
			}

			unsigned long b = 1;

			//make it non blocking
			ioctlsocket(ListenSocket, FIONBIO, &b);
			if (iResult == SOCKET_ERROR) {
				printf("listen failed with error: %d\n", WSAGetLastError());
				closesocket(ListenSocket);
				WSACleanup();
			}

			freeaddrinfo(result);
			return 0;
			}
		//-----------------------------------------------------
		int check_for_new_clients()
			{

			len = sizeof(server);
			// Accept a client socket
			ClientSocket = accept(ListenSocket, (struct sockaddr*)& server, &len);


			if (ClientSocket != INVALID_SOCKET) {
				//save client socket into our struct table
				clients[number_of_clients].ss = ClientSocket;
				clients[number_of_clients].connected = TRUE;
				clients[number_of_clients].address = server;
				//and of course we need a calculator too
				number_of_clients++;

				cout << "New client: " << ClientSocket << endl;



				return number_of_clients-1;

				/*printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(ListenSocket);
				WSACleanup();
				return 1;*/

			}

		
			return -1;
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
		int receive_data(SOCKET socket, char *data, int maxlen)
			{
			int iResult = recv(socket, data, maxlen, 0);
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
			/*if (listen_to_clients())
				return false;*/
			return true;
			}
		//----------------------------------------------------------------------------------------------------------------
		void loop_client(int clientnum)
			{
			if (!clients[clientnum].connected)
				return;
			client_data_packet_ temp;
			server_data_packet_ temp_server_data;
			bool connection_error = FALSE;
			StopWatchMicro_ sw;
			sw.start();
			while (running)
				{
				//cout << "Client " << clients[ii].ss << " still connected" << endl;
				//cout << clientnum<<"    before recv: "<<sw.elapse_micro() << endl;
				int res = receive_data(clients[clientnum].ss, temp.get_address(), temp.get_size());
				//cout << clientnum << "  after recv: " << sw.elapse_micro() << "   res:   " << res<< endl;
				if (res <= 0 && WSAGetLastError() != WSAEWOULDBLOCK)
					{
					connection_error = TRUE;
					break;	
					}
				client_data[clientnum] = temp;					
				this_thread::sleep_for(chrono::milliseconds(5));
				temp_server_data = data_packet;
				//cout << clientnum << "  before send: " << sw.elapse_micro() << endl;
				res = send(clients[clientnum].ss, temp_server_data.get_address(), temp_server_data.get_size(), 0);
				//cout << clientnum << "  after send: " << sw.elapse_micro() << "   res:   " << res << endl;
				if (res == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
					{
					connection_error = TRUE;
					break;
					}
					
				this_thread::sleep_for(chrono::milliseconds(5));
				}

			if (connection_error == TRUE)
				{
				int error = WSAGetLastError();
				//if (error != WSAEWOULDBLOCK) 
				{
					//error or connection closed
					cout << "Client disconnected" << endl;
					cout << "Client error: " << error << endl;
					clients[clientnum].connected = FALSE;
					client_data[clientnum] = client_data_packet_();
					number_of_clients--;
					}
				}
			}
		public:
			void run(int port)
			{
			if (!init_connection(port))
				return;
			vector<thread> clientthreads;
			
			while (running)
				{
				this_thread::sleep_for(chrono::milliseconds(100));
				int clientnum = check_for_new_clients();
				if (clientnum >= 0)
					{
					clientthreads.push_back(thread(&server_::loop_client,this, clientnum));
					}
				}
			for (int i = 0; i < clientthreads.size(); i++)
				clientthreads[i].join();
				

			close_connection();			
			}
	
		server_data_packet_ data_packet;
		client_data_packet_ client_data[MAX_CLIENTS];
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
	void get_incoming_data_packet(client_data_packet_ &data);
	void get_all_incoming_data(client_data_packet_ **data);
	void start_server(int port = 27015);