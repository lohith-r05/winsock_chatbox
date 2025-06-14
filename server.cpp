#include <bits/stdc++.h>
#include <winsock2.h>
#include <thread>
#include <mutex> // mutual exclusion header
#include "database.h" // include for saving messages
using namespace std;

class Server {
    WSADATA wsa; 
    SOCKET serverSocket; 
    sockaddr_in serverAddr;
    vector<SOCKET> clients; // List of connected clients
    map<SOCKET, string> clientNames; // Map to store names of clients
    mutex mtx; // Mutex for thread-safe access to clients vector
    Database db; // database instance

public:
    Server() : db("mydb.db") {
        WSAStartup(MAKEWORD(2, 2), &wsa);
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(8080);

        bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
        listen(serverSocket, SOMAXCONN);

        cout << "Server listening on port 8080...\n";
    }

    // Broadcast message to all clients except the sender
    void broadcast(const string& msg, SOCKET sender) {
        lock_guard<mutex> lock(mtx);
        for (SOCKET client : clients) {
            if (client != sender) {
                send(client, msg.c_str(), msg.size() + 1, 0);
            }
        }
    }

    // Handle client connection
    void handleClient(SOCKET clientSocket) {
        char buffer[1024]; 

        // First receive the client's name
        ZeroMemory(buffer, sizeof(buffer));
        int nameLen = recv(clientSocket, buffer, sizeof(buffer), 0);
        string clientName = (nameLen > 0) ? string(buffer) : "Unknown";

        {
            lock_guard<mutex> lock(mtx);
            clientNames[clientSocket] = clientName;
        }

        cout << clientName << " connected.\n";
        broadcast(clientName + " joined the chat.", clientSocket);

        while (true) {
            ZeroMemory(buffer, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

            // Check if the client has disconnected
            if (bytesReceived <= 0) {
                cout << clientName << " disconnected.\n";

                broadcast(clientName + " left the chat.", clientSocket);

                // Remove client safely
                lock_guard<mutex> lock(mtx);
                auto it = find(clients.begin(), clients.end(), clientSocket);
                if (it != clients.end()) {
                    clients.erase(it);
                }
                clientNames.erase(clientSocket);
                // why mutex for removal is that the clients vector is shared among multiple threads
                closesocket(clientSocket);
                break;
            }

            string msg = buffer;
            cout << clientName << ": " << msg << endl;
            broadcast(clientName + ": " + msg, clientSocket);

            // Save the message to the database with timestamp
            time_t now = time(0);
            string timestamp = ctime(&now);
            timestamp.pop_back(); // remove trailing newline
            db.SaveMessage(timestamp, clientName, msg);
        }
    }

    // Accept new client connections and handle them in separate threads
    void run() {
        while (true) {
            sockaddr_in clientAddr;
            int clientSize = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
            cout << "New client connected.\n";
            // Add the new client to the list of clients with mutex protection
            {
                lock_guard<mutex> lock(mtx);
                clients.push_back(clientSocket);
            }
            // Create a new thread to handle the client 
            // detach() means the thread will run independently, which runs even after the main thread exits
            // detach() is opposite of join(), which waits for the thread to finish
            thread(&Server::handleClient, this, clientSocket).detach();
        }
    }

    ~Server() {
        closesocket(serverSocket);
        WSACleanup();
    }
};

int main() {
    Server server;
    server.run();
    return 0;
}
