#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

class TCPClient {
private:
    int clientSocket;

public:
    TCPClient(const char* serverIp, int port) {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            std::cerr << "Error creating socket\n";
            return;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        if (inet_pton(AF_INET, serverIp, &serverAddr.sin_addr) == -1) {
            std::cerr << "Invalid address/ Address not supported\n";
            close(clientSocket);
            return;
        }

        if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            std::cerr << "Connection failed\n";
            close(clientSocket);
            return;
        }

        std::cout << "Connected to server\n";
    }


    void sendRoomID(int roomID) {
        if (send(clientSocket, &roomID, sizeof(roomID), 0) == -1) {
            std::cerr << "Send failed\n";
            close(clientSocket);
            return;
        }

        int response;
        ssize_t bytesReceived = recv(clientSocket, &response, sizeof(response), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Receive failed\n";
            close(clientSocket);
            return;
        }

        std::cout << "Received from server: " << response << std::endl;
    }

    ~TCPClient() {
        close(clientSocket);
    }
};

int main() {
    const char* serverIp = "127.0.0.1";
    int port = 8080;

    TCPClient client(serverIp, port);

    int roomID;
    std::cout << "Enter room ID: ";
    std::cin >> roomID;

    client.sendRoomID(roomID);

    return 0;
}
