#include <iostream>
#include <winsock2.h>
using namespace std;

class SERVER {
private:
    WSADATA data;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientSize;

public:
    SERVER() : serverSocket(INVALID_SOCKET),
               clientSocket(INVALID_SOCKET),
               clientSize(sizeof(clientAddr)) {}

    bool initialize() {
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }

    bool createSocket() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        return serverSocket != INVALID_SOCKET;
    }

    bool bindSocket() {
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(8080);
        return bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) != SOCKET_ERROR;
    }

    bool listenForConnections() {
        return listen(serverSocket, SOMAXCONN) != SOCKET_ERROR;
    }

    bool acceptClient() {
        cout << "WAITING for CONNECTIONS..." << endl;
        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        return clientSocket != INVALID_SOCKET;
    }

    void receiveMessage() {
        char buffer[4096] = {};
        ZeroMemory(buffer, 4096);
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) cout << "Client says: " << buffer << endl;
    }

    void cleanup() {
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
    }
};

int main() {
    SERVER server;

    if (!server.initialize()) {
        cout << "Initialization failed!" << endl;
    }

    if (!server.createSocket()) {
        cout << "Socket creation failed!" << endl;
    }

    if (!server.bindSocket()) {
        cout << "Binding failed!" << endl;
    }

    if (!server.listenForConnections()) {
        cout << "Listening failed!" << endl;
    }

    if (!server.acceptClient()) {
        cout << "Accepting client failed!" << endl;
    }

    server.receiveMessage();
    server.cleanup();

    return 0;
}
