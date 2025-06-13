#include <iostream>
#include <winsock2.h>

using namespace std;

class Client {
    WSADATA wsa;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

public:
    Client(const char* serverIP, int port) {
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            cerr << "WSAStartup failed.\n";
            exit(1);
        }

        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Socket creation failed.\n";
            WSACleanup();
            exit(1);
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(serverIP);

        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Connection to server failed.\n";
            closesocket(clientSocket);
            WSACleanup();
            exit(1);
        }

        cout << "Connected to server.\n";
    }

    void sendMessage(const string& msg) {
        send(clientSocket, msg.c_str(), msg.size() + 1, 0);
    }

    ~Client() {
        closesocket(clientSocket);
        WSACleanup();
    }
};

int main() {
    Client client("192.168.35.108", 8080); // Connect to localhost

    string message;
    cout << "Enter the content to send to server: ";
    getline(cin, message);

    client.sendMessage(message);

    return 0;
}
