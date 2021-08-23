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
string dataPath = "/home/nikola/partitions/soc-karate/N4_K1";

//  Command-line arg: Number of workers, WorkerId, Other

int main(int argc, char* argv[])
{
    int id = atoi(argv[2]);
    int numWorkers = atoi(argv[1]);

    Worker w(id, numWorkers);
    w.setWorkersSockAddr(); // set sockaddr of other workers
    w.LoadNodesData(dataPath); // load graph parition based on id

    // Worker broadcasts node that it has to other workers
    // and receaves info from other nodes
    thread sendBroadCastTr(Worker::broadcastNodeInfo, w);
    Worker::recvWorkersNodeInfo(w);
    sendBroadCastTr.join();

    /*thread clusteringCoeffTr(Worker::calculateClusteringCoeff, ref(w));

    clusteringCoeffTr.join();*/

    thread recvNodeNeighborsRequestTr(Worker::recvNodeNeighborsRequest, w);

    Worker::calculateClusteringCoeff(w);

    recvNodeNeighborsRequestTr.join();

    cout << endl;
}