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

string partitionsDir = "fb-pages-food/N2_K1/";
string dataPath = "/home/nikola/partitions/" + partitionsDir;
string resultsPath = "/home/nikola/results/" + partitionsDir;
string ipFileName = "/home/nikola/ip_addrs.txt";

//  Command-line arg: Number of workers, WorkerId, Other

int main(int argc, char* argv[])
{
    chrono::steady_clock::time_point startTime = chrono::steady_clock::now();

    int id = atoi(argv[2]);
    int numWorkers = atoi(argv[1]);

    // Init phase

    Worker w(id, numWorkers);
    w.createAndBindSock(SOCK_STREAM);
    w.setWorkersSockAddr(ipFileName); // set sockaddr of other workers
    w.LoadNodesData(dataPath); // load graph partition based on id

    // Worker broadcasts nodes that it has to other workers
    // and receives info about other nodes. TCP is used

    thread broadcastNodeInfoTr(Worker::broadcastNodeInfo, w);
    Worker::recvNodeInfo(w);
    broadcastNodeInfoTr.join();

    // ------------------

    w.createAndBindSock(SOCK_DGRAM);
    sleep(2);

    thread listenForRequestTr(Worker::listenForRequest, ref(w));
    
    Worker::calculateClusteringCoeff(w);

    cout << "Worker " << id << " finished" << endl;
    auto endTime = chrono::steady_clock::now();
    int executionTime = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
    cout << "Worker " << w.getId() << " time: " << executionTime << " ms" << endl;


    if (Worker::broadcastWorkConsensus(w)) {
        cout << "Worker " << id << " shutting down" << endl;
        w.LogResults(w, resultsPath, executionTime);

        exit(EXIT_SUCCESS);
    }

    listenForRequestTr.join();

    cout << "Worker " << id << " shutting down" << endl;
    w.LogResults(w, resultsPath, executionTime);
}