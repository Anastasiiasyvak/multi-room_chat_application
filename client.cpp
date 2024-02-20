#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>


class TCPClient{
public:
    TCPClient(const char* serverIp, int port){
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            perror("Error creating socket");
            exit(1);
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIp, &(serverAddr.sin_addr));
    }

    ~TCPClient() {
        close(clientSocket);
    }

    bool connectToServer() {
        return connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) != -1;
    }


    void receiveServerMessage() {
        char fileMessage[1024];
        memset(fileMessage, 0, sizeof(fileMessage));
        ssize_t bytes = recv(clientSocket, fileMessage, sizeof(fileMessage), 0);
        if (bytes > 0) {
            std::cout << fileMessage << std::endl;

        } else {
            std::cerr << "Failed to receive message from server." << std::endl;
        }
    }

    void getName() const {
        std::string clientName;
        std::cout << "Enter client name: ";
        getline(std::cin, clientName);
        send(clientSocket, clientName.c_str(), clientName.size(), 0);
    }

    void getRoomID() const {
        std::string roomId;
        std::cout << "Enter room id: ";
        while (!(std::cin >> roomId)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter an integer: ";
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        send(clientSocket, roomId.c_str(), roomId.size(), 0);
    };


    void messaging(){
        std::string message;
        while (true) {
            std::thread threadForMessagesReceiving(&TCPClient::receiveServerMessage, this);
            threadForMessagesReceiving.detach();
            std::cout << "Enter the message: ";
            std::getline(std::cin, message);
            if (message == "STOP") {
                break;
            }
            send(clientSocket, message.c_str(), message.size(), 0);
        }
    }

private:
    int clientSocket;
    sockaddr_in serverAddr;

};

int main() {
    int port = 12345;
    const char* serverIp = "127.0.0.1";

    TCPClient client(serverIp, port);

    if (!client.connectToServer()) {
        perror("Connect failed");
        return 1;
    }

    client.getName();
    client.getRoomID();
    client.messaging();
    return 0;
}