#pragma once

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

class Worker
{
	protected:

		int id;
		int numWorkers;
		int sockfd;
		sockaddr_in sockAddr;
		map<int, vector<int>> nodes;
		vector<int> workersSockfd;
	
	public:
	
		Worker(int WorkerId, int numWorkers);

		int getId() { return id; }
		int getNumWorkers() { return numWorkers; }
		int getSockfd() { return sockfd; }
		sockaddr_in getSockAddr() { return sockAddr; }
		map<int, vector<int>> getNodes() { return nodes; }
		vector<int> getWorkersSockfd() { return workersSockfd; }

		void initWorkerSockets();
		void LoadNodesData(string path);
};