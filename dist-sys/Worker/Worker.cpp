#include "Worker.h"

Worker::Worker() {
	this->id = 0;
	this->address = "127.0.0.1";
	this->port = 8080;
	this->nodes = map<int, vector<int>>();
	this->workerAddr = vector<string>();
	this->workerPort = vector<int>();
}

Worker::Worker(int id, string address, int port, map<int, vector<int>> nodes,
	vector<string> workerAddr, vector<int> workerPort) {

	this->id = id;
	this->address = address;
	this->port = port;
	this->nodes = nodes;
	this->workerAddr = workerAddr;
	this->workerPort = workerPort;
}

void Worker::setId(int id) { this->id = id; }
void Worker::setAddress(string address) { this->address = address; }
void Worker::setPort(int port) { this->port = port; }
void Worker::setNodes(map<int, vector<int>> nodes) { this->nodes = nodes; }
void Worker::setWorkerAddr(vector<string> workerAddr) { this->workerAddr = workerAddr; }
void Worker::setWorkerPort(vector<int> workerPort) { this->workerPort = workerPort; }

void Worker::GenerateWorkerPort() {
	int portPrefix = 8080;
	port = portPrefix + id;
}

void Worker::LoadNodesData(string path) {
	std::string line;
	int node = 0;
	std::vector<int> nodeNeighbors;

	path = path + "/" + std::to_string(id) + ".txt";
	std::ifstream f(path);

	while (std::getline(f, line)) {
		string str1 = line.substr(0, line.find(",")); // str1 holds node
		string str2 = line.substr(line.find(",") + 2); // str2 holds neighbors of node
		str2.erase(std::remove(str2.begin(), str2.end(), '\r'), str2.end()); // remove /r from string

		node = std::stoi(str1);

		std::stringstream ss(str2);
		std::string tmp;

		while (std::getline(ss, tmp, ' '))
		{
			nodeNeighbors.push_back(std::stoi(tmp)); // save node neighbor to vector
		}

		nodes.insert(std::pair<int, std::vector<int>>(node, nodeNeighbors));
		nodeNeighbors.clear();

	}
}
