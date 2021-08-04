#pragma once

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <thread>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#define BUFFERSIZE 1024

using namespace std;

class Worker
{
	protected:

		int id;
		int numWorkers;
		int sockfd;
		sockaddr_in sockAddr;
		map<int, vector<int>> nodes;
		map<int, vector<int>> otherWorkersNodes;
		vector<sockaddr_in> workersSockAddr;
	
	public:
	
		Worker(int WorkerId, int numWorkers);

		int getId() { return id; }
		int getNumWorkers() { return numWorkers; }
		int getSockfd() { return sockfd; }
		sockaddr_in getSockAddr() { return sockAddr; }
		map<int, vector<int>> getNodes() { return nodes; }
		map<int, vector<int>> getOtherWorkersNodes() { return otherWorkersNodes; }
		vector<sockaddr_in> getWorkersSockAddr() { return workersSockAddr; }

		void setWorkersSockAddr();
		void addToOtherWorkersNodes(pair<int, vector<int>> p);
		void LoadNodesData(string path);

		static void broadcastNodeInfo(Worker w);
		static void sendDataToWorker(Worker w, int workerId, int* data, int dataLen);
		static void recvWorkersNodeInfo(Worker& w);
};