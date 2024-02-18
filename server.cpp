#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <algorithm>

class TCPServer{
public:

    struct Client {
        int socket;
        int roomID;
        int clientID;
    };

    std::vector<Client> clients;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int clientCounter = 0;

    static void* handleClientHelper(void* arg) {
        TCPServer* server = static_cast<TCPServer*>(arg);
        return server->handleClient();
    }

    void* handleClient() {
        int clientSocket = clients.back().socket;

        int roomID;
        int bytesReceived = recv(clientSocket, &roomID, sizeof(roomID), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Failed to receive room ID from client.\n";
            close(clientSocket);
            pthread_exit(nullptr);
        }

        pthread_mutex_lock(&mutex);
        clientCounter++;
        int clientID = clientCounter;
        pthread_mutex_unlock(&mutex);

        clients.push_back({clientSocket, roomID, clientID});

        std::cout << "Client " << clientID << " joined room " << roomID << std::endl;

        char buffer[4096];
        while (true) {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) {
                pthread_mutex_lock(&mutex);
                auto it = std::find_if(clients.begin(), clients.end(), [&](const Client& c) {
                    return c.socket == clientSocket;
                });
                if (it != clients.end()) {
                    clients.erase(it);
                }
                pthread_mutex_unlock(&mutex);
                std::cout << "Client " << clientID << " disconnected.\n";
                close(clientSocket);
                pthread_exit(nullptr);
            }
            buffer[bytesReceived] = '\0';
            std::cout << "Client " << clientID << " message: " << buffer << std::endl;

            pthread_mutex_lock(&mutex);
            for (const auto& client : clients) {
                if (client.roomID == roomID && client.socket != clientSocket) {
                    if (send(client.socket, buffer, bytesReceived, 0) == -1) {
                        std::cerr << "Failed to send message to client.\n";
                    }
                }
            }
            pthread_mutex_unlock(&mutex);
        }

    }

    void startServer(){
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            std::cerr << "Socket creation failed.\n";
            return;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(8080);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            std::cerr << "Bind failed.\n";
            close(serverSocket);
            return;
        }

        if (listen(serverSocket, SOMAXCONN) == -1) {
            std::cerr << "Listen failed.\n";
            close(serverSocket);
            return;
        }

        std::cout << "Server is listening on port 8080...\n";

        while (true) {
            sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket == -1) {
                std::cerr << "Accept failed.\n";
                close(serverSocket);
                return;
            }

            pthread_mutex_lock(&mutex);
            clients.push_back({clientSocket, -1, -1});
            pthread_mutex_unlock(&mutex);

            pthread_t thread;
            if (pthread_create(&thread, nullptr, &TCPServer::handleClientHelper, this) != 0) {
                std::cerr << "Failed to create thread.\n";
                close(clientSocket);
            }
            pthread_detach(thread);
        }

    }
};



int main() {
    TCPServer server;
    server.startServer();
    return 0;
}
