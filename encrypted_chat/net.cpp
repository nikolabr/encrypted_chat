#include "net.hpp"

void WinsockInterface::WinsockInit() {
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed with error: " << iResult << std::endl;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
}

int WinsockInterface::Resolve() {
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
        WSACleanup();
        return 1;
    }
}

int WinsockInterface::SetupSocket() {
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cout << "listen failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cout << "listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        std::cout << "accept failed with error: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
    }

    // No longer need server socket
    closesocket(ListenSocket);
}

WinsockInterface::WinsockInterface() {
	WinsockInit();
    Resolve();
    SetupSocket();
}

void WinsockInterface::Receive() {
    // Receive until the peer shuts down the connection
    std::cout << "Receiving!" << std::endl;
    do {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            std::cout << "Bytes received: " << iResult << std::endl;

            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
                closesocket(ClientSocket);
                WSACleanup();
            }
            std::cout << "Bytes sent: " << iSendResult << std::endl;
        }
        else if (iResult == 0)
            std::cout << "Connection closing..." << std::endl;
        else {
            std::cout << "recv failed with error: %d\n" << WSAGetLastError() << std::endl;
            closesocket(ClientSocket);
            WSACleanup();
        }

    } while (iResult > 0);
}
