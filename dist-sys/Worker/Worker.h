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
#include <mutex>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#define SIZE 8192
#define FIX_PORT 50000
#define SLEEP_PERIOD 1500000

#define NEIGHREQ 0		// requesting for node neighbors message
#define CONS 1			// consensus message
#define CALCNODE 2		// message that holds clustering coeff of node calculated
#define STATUS 3		// message holding status of other workers

enum Status { ACTIVE = 0, CRASH = 1, CRASH_RECOVERY = 2 };

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
		vector<bool> workConsensus;					// stores which workers have agreed to end execution
		vector<Status> workersStatus;				// stores which workers are active (have not crashed)
		vector<int> timeCheckpoints;				// used for capturing execution time of each part of process
		int numMessages;							// count total number of messages worker has sent
		Status stat;								// Status of worker object
	
	public:
		static mutex mtx;							// mutex is used so race condition wouldn't occur

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
		vector<Status>& getWorkersStatus() { return workersStatus; }
		vector<int>& getTimeCheckpoint() { return timeCheckpoints; }
		int getNumMessages() { return numMessages; }
		Status& getStatus() { return stat; }

		void createAndBindSock(int type);
		void setWorkersSockAddr(string ipFileName);
		void loadNodesData(string path);

		static void setSockOpt(int sock, int sec, int microsec);

		static void broadcastNodeInfo(Worker w);
		static void recvNodeInfo(Worker& w);

		static void sendNodeInfoToWorker(Worker w, int workerId, int* data, int dataLen);
		static vector<int> requestNodeNeighbors(Worker& w, int node);
		static void listenForRequest(Worker& w);
		static int calculateClusteringCoeff(Worker& w, int faultCounter = 0);
		static void broadcastClusteringCoeffInfo(Worker& w, int node, float clusteringCoeff);
		static void broadcastWorkersStatus(Worker& w);
		static bool broadcastWorkConsensus(Worker& w);
		static void sendDataToWorker(Worker w, int workerId, int* data, int dataLen);
		static bool checkWorkConsensus(Worker w);

		void addTimeCheckpoint(chrono::steady_clock::time_point& startTime);
		int totalTime();
		void incNumMessages(int val = 1);
		static void logResults(Worker w, string path);
};