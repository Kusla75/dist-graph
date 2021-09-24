#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <cstdlib>

#include "Worker.h"

using namespace std;

string homeDir = getenv("HOME");

// fb-pages-food/N4_K1_ ...
//  Command-line arg: Number of workers, WorkerId, Partitions path, Other

int main(int argc, char* argv[])
{
    string partitionsDir = argv[3];
    string dataPath = homeDir + "/partitions/" + partitionsDir;
    string resultsPath = homeDir + "/results/" + partitionsDir;
    string ipFileName = homeDir + "/ip_addrs.txt";


    chrono::steady_clock::time_point startTime = chrono::steady_clock::now();

    int id = atoi(argv[2]);
    int numWorkers = atoi(argv[1]);
    mutex mtx;

    // Init phase

    Worker w(id, numWorkers);
    w.createAndBindSock(SOCK_STREAM);
    w.setWorkersSockAddr(ipFileName); // set sockaddr of other workers
    w.loadNodesData(dataPath); // load graph partition based on id

    // Worker broadcasts nodes that it has to other workers
    // and receives info about other nodes. TCP is used

    thread broadcastNodeInfoTr(Worker::broadcastNodeInfo, w);
    Worker::recvNodeInfo(w);
    broadcastNodeInfoTr.join();

    w.createAndBindSock(SOCK_DGRAM);
    usleep(SLEEP_PERIOD);

    cout << "Node info broadcasted" << endl; // Debug

    w.addTimeCheckpoint(startTime);
    // ------------------

    thread listenForRequestTr(Worker::listenForRequest, ref(w));
    
    Worker::calculateClusteringCoeff(w);

    w.addTimeCheckpoint(startTime);
    cout << "Worker " << w.getId() << " calculating time: " << w.getTimeCheckpoint().back() << " ms" << endl;

    Worker::broadcastWorkConsensus(w);
    w.addTimeCheckpoint(startTime);

    listenForRequestTr.join();

    w.LogResults(w, resultsPath);
}