#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>

#include "Worker.h"

using namespace std;
string dataPath = "/home/nikola/partitions/N4_K1";

//  Command-line arg: Number of workers, WorkerId, Other

int main(int argc, char* argv[])
{
    int workerId = atoi(argv[2]);
    int numWorkers = atoi(argv[1]);

    Worker w(workerId, numWorkers);
    w.setWorkersSockAddr();
    w.LoadNodesData(dataPath);

    int buffer[1024];
    sockaddr_in address = w.getSockAddr();
    int addrlen = sizeof(w.getSockAddr());
    int new_socket;

    thread tr(Worker::broadcastNodeInfo, w);

    if (listen(w.getSockfd(), 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    for (int n = 0; n < w.getNumWorkers()-1; ++n)
    {
        if ((new_socket = accept(w.getSockfd(), (struct sockaddr*)&address,
            (socklen_t*)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        int byteSize = recv(new_socket, buffer, 1024, 0);
        int len = byteSize / sizeof(int);

        cout << "Node: " << buffer[0] << endl;
        for (int i = 1; i < len; ++i) {
            cout << buffer[i] << " ";
        }
        cout << "\n" << endl;
        close(new_socket);
    }

    tr.join();
}