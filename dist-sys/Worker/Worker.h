#pragma once

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace std;

class Worker
{
	protected:

		int id;
		string address;
		int port;
		map<int, vector<int>> nodes;
		vector<string> workerAddr;
		vector<int> workerPort;

	public:

		Worker();
		Worker(int id, string address, int port, map<int, vector<int>> nodes,
			vector<string> workerAddr, vector<int> workerPort);

		int getId() { return id; }
		string getAddress() { return address; }
		int getPort() { return port; }
		map<int, vector<int>> getNodes() { return nodes; }
		vector<string> getWorkerAddr() { return workerAddr; }
		vector<int> getWorkerPort() { return workerPort; }

		void setId(int id);
		void setAddress(string address);
		void setPort(int port);
		void setNodes(map<int, vector<int>> nodes);
		void setWorkerAddr(vector<string> workerAddr);
		void setWorkerPort(vector<int> workerPort);

		void GenerateWorkerPort();
		void LoadNodesData(string path);
};



