#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <mutex>
//#include "ground.h"
#define DATA_INT 256
#define DATA_FLOAT 2048
class server_data_packet_
	{ 
	private:
		std::mutex m;
		void lock()
			{
			m.lock();
			}
		void unlock()
			{
			m.unlock();
			}
	public:
		int dataint[DATA_INT];
		float datafloat[DATA_FLOAT];
		void operator=(server_data_packet_ &rhs)
			{
			lock();
			rhs.lock();
			for (int ii = 0; ii < DATA_FLOAT; ii++)
				datafloat[ii] = rhs.datafloat[ii];
			for (int ii = 0; ii < DATA_INT; ii++)
				dataint[ii] = rhs.dataint[ii];
			rhs.unlock();
			unlock();
			}
		server_data_packet_()
			{
			for (int ii = 0; ii < DATA_INT; ii++)
				dataint[ii] = 0;
			for (int ii = 0; ii < DATA_FLOAT; ii++)
				datafloat[ii] = 0.0;
			}
		//-------------------------------------------------------------------------
		int get_size()
			{
			return DATA_INT * sizeof(int) + DATA_FLOAT * sizeof(float);
			}
		char *get_address()
			{
			char *addr = (char*)dataint;
			return addr;
			}

	};

#define CLIENT_DATA_INT 16
#define CLIENT_DATA_FLOAT 32
//#define CLIENT_DATA_INT 256
//#define CLIENT_DATA_FLOAT 1024
class client_data_packet_
	{
	private:
		std::mutex m;
		void lock()
			{
			m.lock();
			}
		void unlock()
			{
			m.unlock();
			}
	public:
		int dataint[CLIENT_DATA_INT];
		float datafloat[CLIENT_DATA_FLOAT];
		void operator=(client_data_packet_ &rhs)
			{
			lock();
			rhs.lock();
			for (int ii = 0; ii < CLIENT_DATA_FLOAT; ii++)
				datafloat[ii] = rhs.datafloat[ii];
			for (int ii = 0; ii < CLIENT_DATA_INT; ii++)
				dataint[ii] = rhs.dataint[ii];
			rhs.unlock();
			unlock();
			}
		client_data_packet_()
			{
			for (int ii = 0; ii < CLIENT_DATA_INT; ii++)
				dataint[ii] = 0;
			for (int ii = 0; ii < CLIENT_DATA_FLOAT; ii++)
				datafloat[ii] = 0.0;
			}
		//-------------------------------------------------------------------------
		int get_size()
			{
			return CLIENT_DATA_INT * sizeof(int) + CLIENT_DATA_FLOAT * sizeof(float);
			}
		char *get_address()
			{
			char *addr = (char*)dataint;
			return addr;
			}

	};