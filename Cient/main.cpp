#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <cstdlib>
#pragma comment(lib, "Ws2_32.lib")

std::string getMovement(){
    std::string input;
    std::cout << "Enter movement direction (W/A/S/D): " << std::endl;
    std::cin >> input;

    // Validate input
    if (input.length() == 1) {
        return input;
    }else {
        std::cerr << "Invalid input. Please enter a single character." << std::endl;
        // Handle the error, for example, by returning a default value or asking the user to input again.
        return '\0';  // Returning null character as an indication of an error
    }
}

void sendData(SOCKET sock, bool pHraBezi) {

    while(pHraBezi) {
        std::string input = "" + getMovement();
        std::cout << input;
        // Send a message to the server
        if (input.compare("Q") == 0) {
            break;
        }
        const char *messageToSend = input.c_str();
        if (send(sock, messageToSend, strlen(messageToSend), 0) == SOCKET_ERROR) {
            std::cerr << "Chyba pri odosielaní správy na server." << std::endl;
            closesocket(sock);
            WSACleanup();
            break;
        }
    }
}

void receiveData(SOCKET sock, bool pHraBezi) {

    const int intervalInSeconds = 2;
    clock_t lastExecutionTime = clock();

    while(pHraBezi) {
        clock_t currentTime = clock();
        double elapsedSeconds = static_cast<double>(currentTime - lastExecutionTime) / CLOCKS_PER_SEC;

        if (elapsedSeconds >= intervalInSeconds) {

            // Receive a response from the server
            char buffer[4096];
            for (int i = 0; i < 4096; ++i) {
                buffer[i] = ' ';
            }

            int bytesRead = recv(sock, buffer, sizeof(buffer), 0);

            if (bytesRead <= 0) {
                std::cerr << "Chyba pri prijimani odpovede od servera." << std::endl;
                break;
            } else {
                buffer[bytesRead] = '\0';  // Null-terminate the received data
                std::cout << buffer << std::endl;
            }
            lastExecutionTime = currentTime;
        }
    }
}


int main() {
    bool hraBezi = true;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Chyba pri inicializacii Winsock." << std::endl;
        return EXIT_FAILURE;
    }

    // Create a socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Chyba pri vytvarana soketu." << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Set up the server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8008);  // Use the same port as the server
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Chyba pri pripájaní k serveru." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    std::cout << "Pripojený k serveru." << std::endl;

    std::thread senderThread(sendData, clientSocket, hraBezi);
    std::thread receiverThread(receiveData, clientSocket, hraBezi);

    senderThread.join();
    receiverThread.join();

    // Clean up
    closesocket(clientSocket);
    WSACleanup();
    return EXIT_SUCCESS;
}
