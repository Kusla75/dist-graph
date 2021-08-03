#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "Worker.h"
#define PORT 8080

int main(int argc, char* argv[])
{
    if (atoi(argv[2]) == 0)
    {
        string dataPath = "/home/nikola/partitions/N3_K1";

        Worker w;
        int sockfd = 0;
        sockaddr_in server_addr;
        string message = "Hello from " + std::to_string(w.getId());

        w.setId(atoi(argv[1]));
        w.GenerateWorkerPort();
        w.LoadNodesData(dataPath);

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cout << "Socket creation error" << std::endl;
            return -1;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = w.getPort();

        if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
        {
            printf("\nInvalid address/ Address not supported \n");
            return -1;
        }

        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        {
            printf("\nConnection Failed \n");
            return -1;
        }

        send(sockfd, &message[0], message.length(), 0);
        return 0;
    }
    else {
        int server_fd, new_socket, valread;
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);
        char buffer[1024] = { 0 };

        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Forcefully attaching socket to the port 8080
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
            &opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = PORT;

        // Forcefully attaching socket to the port 8080
        if (bind(server_fd, (struct sockaddr*)&address,
            sizeof(address)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(server_fd, 3) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
            (socklen_t*)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        valread = read(new_socket, buffer, 1024);
        printf("%s\n", buffer);
        return 0;
    }
}