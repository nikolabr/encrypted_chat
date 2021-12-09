#pragma once


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#include "crypto.hpp"

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN CIPHER_LEN
#define DEFAULT_PORT "28015"

class WinsockInterface {
	WSADATA wsaData; 

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	
public: 
	int iResult, iSendResult;

	WinsockInterface();
	void WinsockInit();
	int Resolve();
	int SetupSocket();
	void Receive();


};