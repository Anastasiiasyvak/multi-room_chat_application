#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <algorithm>
#include <sys/socket.h>
#include <pthread.h>

struct Message {
    std::string content;
    std::string clientName;
    int clientSocket;
    int id;
};

class LinuxNetworkSystem {
public:
    LinuxNetworkSystem() {
        mutex = PTHREAD_MUTEX_INITIALIZER;
    }

    int createSocket(int domain, int type, int protocol) {
        return socket(domain, type, protocol);
    }

    int bindSocket(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
        return bind(sockfd, addr, addrlen);
    }

    int listenSocket(int sockfd, int backlog) {
        return listen(sockfd, backlog);
    }

    int acceptConnection(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
        return accept(sockfd, addr, addrlen);
    }

    ssize_t receiveData(int sockfd, void *buf, size_t len, int flags) {
        return recv(sockfd, buf, len, flags);
    }

    ssize_t sendData(int sockfd, const void *buf, size_t len, int flags) {
        return send(sockfd, buf, len, flags);
    }

    int closeSocket(int sockfd) {
        return close(sockfd);
    }

    void error(const char* message) const {
        pthread_mutex_lock(&mutex);
        perror(message);
        pthread_mutex_unlock(&mutex);
    }

private:
    mutable pthread_mutex_t mutex;
};

class Room {
public:
    std::string name;
    std::thread roomThread;
    std::vector<int> clients;
    std::queue<Message> messageQueue;
    mutable std::mutex mutex;
    std::condition_variable messageCondition;
    int ID = 0;
    LinuxNetworkSystem linuxNetworkSystem;

    Room(std::string name) : name(name) {
        roomThread = std::thread(&Room::broadcastMessages, this);
    }

    void addClient(int clientSocket) {
        std::lock_guard<std::mutex> lock(mutex);
        clients.push_back(clientSocket);
        std::cout << "Client " << clientSocket << " joined room " << name << std::endl;
    }

    void deleteClient(int clientSocket) {
        std::lock_guard<std::mutex> lock(mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
        std::cout << "Client " << clientSocket << " left the room " << name << std::endl;
    }

    void addMessageToQueue(const Message& message) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            messageQueue.push(message);
        }
        messageCondition.notify_one();
    }

    std::string getNameRoom() const {
        return name;
    }

    void broadcastMessages() {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mutex);
            messageCondition.wait(lock, [this]{ return !messageQueue.empty();});
            while (!messageQueue.empty())
            {
                Message message = messageQueue.front();
                messageQueue.pop();
                lock.unlock();
                for (int clientSocket : clients) {
                    if (clientSocket != message.clientSocket){
                        std::string messageContentName = "\n" + message.clientName + ": " + message.content;
                        linuxNetworkSystem.sendData(clientSocket, messageContentName.c_str(), messageContentName.length(), 0);
                    }
                }
                lock.lock();
            }
            if (clients.empty()){
                break;
            }
        }
    }
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

class TCPServer{
private:
    int port = 12346;
    sockaddr_in clientAddr;
    LinuxNetworkSystem linuxNetworkSystem;
    std::vector<std::thread> clientThreads;
    std::vector<std::unique_ptr<Room>> rooms;
    std::mutex roomsMutex;

    void listeningSocket(){
        int serverSocket = linuxNetworkSystem.createSocket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            linuxNetworkSystem.error("Error creating socket");
            return;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (linuxNetworkSystem.bindSocket(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            linuxNetworkSystem.error("Bind failed");
            linuxNetworkSystem.closeSocket(serverSocket);
            return;
        }

        if (linuxNetworkSystem.listenSocket(serverSocket, SOMAXCONN) == -1) {
            return;
        } else {
            std::cout << "Server listening on port " << port << std::endl;

            while (true) {
                socklen_t clientAddrLen = sizeof(clientAddr);
                int clientSocket = linuxNetworkSystem.acceptConnection(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
                if (clientSocket == -1) {
                    linuxNetworkSystem.error("Error accepting client connection");
                    break;
                }

                pthread_mutex_lock(&mutex);
                std::cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
                pthread_mutex_unlock(&mutex);

                clientThreads.emplace_back(&TCPServer::handleCommands, this, clientSocket);
            }
        }

        linuxNetworkSystem.closeSocket(serverSocket);
    }

public:
    TCPServer() : linuxNetworkSystem() {
        listeningSocket();
    }

    ~TCPServer() {
        for (auto& thread : clientThreads) {
            thread.join();
        }
    }

    void getRoomId(int clientSocket, std::string& clientName, std::string& roomName){
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRead = linuxNetworkSystem.receiveData(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cerr << "Failed to read client's name.\n";
            close(clientSocket);
            return;
        }
        clientName = std::string(buffer, bytesRead);

        memset(buffer, 0, sizeof(buffer));
        bytesRead = linuxNetworkSystem.receiveData(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cerr << "Failed to read room name.\n";
            close(clientSocket);
            return;
        }
        roomName = std::string(buffer, bytesRead);
    }

    Room* addRoom(const std::string& roomName) {
        std::lock_guard<std::mutex> lock(roomsMutex);
        for (auto& room : rooms) {
            if (room->getNameRoom() == roomName) {
                return room.get();
            }
        }
        rooms.emplace_back(std::make_unique<Room>(roomName));
        return rooms.back().get();
    }

    void handleCommands(int clientSocket) {
        std::string roomName;
        std::string clientName;
        getRoomId(clientSocket, clientName, roomName);

        Room *room = addRoom(roomName);
        room->addClient(clientSocket);

        while (true) {
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));
            ssize_t receivedBytes = linuxNetworkSystem.receiveData(clientSocket, buffer, sizeof(buffer), 0);
            if (receivedBytes > 0) {
                std::string content(buffer, receivedBytes);
                if (content == "STOP") {
                    room->deleteClient(clientSocket);
                    std::cout << "Client " << clientSocket << " left the room " << roomName << std::endl;
                } else if (content.substr(0, 6) == "REJOIN") {
                    std::string newRoomName = content.substr(7);
                    Room *newRoom = addRoom(newRoomName);
                    if (newRoom != room) {
                        room->deleteClient(clientSocket);
                        newRoom->addClient(clientSocket);
                        room = newRoom;
                        roomName = newRoomName;
                    }
                } else {
                    Message message{content, clientName, clientSocket, room->ID++};
                    room->addMessageToQueue(message);
                }
            } else {
                linuxNetworkSystem.error("Received failed.");
                break;
            }
        }

        room->deleteClient(clientSocket);
        linuxNetworkSystem.closeSocket(clientSocket);
    }

};


int main() {
    TCPServer server;
    return 0;
}