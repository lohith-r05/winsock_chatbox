#include <iostream>
#include <winsock2.h>
#include <thread>
using namespace std;

class Client {
    WSADATA wsa;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    string name;

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
        cout << "Enter your name: ";
        getline(cin, name);
    }

    // Function to receive messages from server
    void receiveLoop() {
        char buffer[1024];
        while (true) {
            ZeroMemory(buffer, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) {
                cout << "Disconnected from server.\n";
                break;
            }
            cout << buffer << endl;
        }
    }

    // Function to send messages to server
    void sendLoop() {
        string msg;
        while (true) {
            getline(cin, msg);
            string fullMessage = name + ": " + msg;
            send(clientSocket, fullMessage.c_str(), fullMessage.size() + 1, 0);
        }
    }

    void startChat() {
        thread receiveThread(&Client::receiveLoop, this);
        thread sendThread(&Client::sendLoop, this);

        // Wait for both threads to finish (they typically won't unless server disconnects)
        receiveThread.join();
        sendThread.join();
    }

    ~Client() {
        closesocket(clientSocket);
        WSACleanup();
    }
};

int main() {
    Client client("192.168.35.108", 8080); // Replace with your server IP if needed
    client.startChat();
    return 0;
}
