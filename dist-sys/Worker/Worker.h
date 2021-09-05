#pragma once

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#define SIZE 8192
#define FIX_PORT 50000

#define NEIGHREQ 0		// requesting for node neighbors message
#define CONS 1			// consensus message
#define CALCNODE 2		// message that holdes clustering coeff of node calculated

using namespace std;

class Worker
{
	protected:

		int id;										// id of worker
		int numWorkers;								// total number of workers in dist system
		int sockfd;									// file descriptor of socket binded to localhost
		sockaddr_in sockAddr;						// struct that holds listening port and address family
		map<int, vector<int>> nodes;				// data that will be used for computation. Holds node and it's neighbors
		map<int, float> clusteringCoeff;			// map that stores computation results (clustering coefficients of each node)
		map<int, vector<int>> otherWorkersNodes;	// map that stores other node locations (which worker has which node)
		vector<sockaddr_in> workersSockAddr;		// socket addresses of other workes
		vector<bool> workConsensus;
	
	public:
	
		Worker(int WorkerId, int numWorkers);

		int getId() { return id; }
		int getNumWorkers() { return numWorkers; }
		int getSockfd() { return sockfd; }
		sockaddr_in& getSockAddr() { return sockAddr; }
		map<int, vector<int>>& getNodes() { return nodes; }
		map<int, float>& getClusteringCoeff() { return clusteringCoeff; }
		map<int, vector<int>>& getOtherWorkersNodes() { return otherWorkersNodes; }
		vector<sockaddr_in>& getWorkersSockAddr() { return workersSockAddr; }
		vector<bool>& getWorkConsensus() { return workConsensus; }

		void createAndBindSock(int type);
		void setWorkersSockAddr();
		void LoadNodesData(string path);

		static void broadcastNodeInfo(Worker w);
		static void recvNodeInfo(Worker& w);

		static void sendNodeInfoToWorker(Worker w, int workerId, int* data, int dataLen);
		static vector<int> requestNodeNeighbors(Worker w, int node);
		static void listenForRequest(Worker& w);
		static void calculateClusteringCoeff(Worker& w);
		static void broadcastClusteringCoeffInfo(Worker& w, int node, float clusteringCoeff);
		static bool broadcastWorkConsensus(Worker& w);
		static void sendDataToWorker(Worker w, int workerId, int* data, int dataLen);
		static bool checkWorkConsensus(Worker w);

		static void LogResults(Worker w, string path, int executionTime);
};