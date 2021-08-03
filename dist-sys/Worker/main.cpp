#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "Worker.h"
#define PORT 8080

string dataPath = "/home/nikola/partitions/N3_K1";

//  Command line arg: Number of workers, WorkerId, Other

int main(int argc, char* argv[])
{
    int workerId = atoi(argv[2]);
    int numWorkers = atoi(argv[1]);

    Worker w(workerId, numWorkers);
    w.setWorkersSockAddr();
    w.LoadNodesData(dataPath);

    if (workerId == 0) {
        w.broadcastNodeInfo();
    }
    else {
        char buffer[1024] = { 0 };
        string message = "Hi from " + std::to_string(w.getId());
        sockaddr_in address = w.getSockAddr();
        int addrlen = sizeof(w.getSockAddr());
        int new_socket;

        if (listen(w.getSockfd(), 3) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        if ((new_socket = accept(w.getSockfd(), (struct sockaddr*)&address,
            (socklen_t*)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        read(new_socket, buffer, 1024);
        send(new_socket, &message[0], message.length(), 0);
        printf("%s\n", buffer);
    }
}