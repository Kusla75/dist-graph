#include "Worker.h"

Worker::Worker(int workerId, int numWorkers) {
	this->id = workerId;
	this->numWorkers = numWorkers;

	// Bind socket init ------

	this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->sockfd == 0) {
		cout << "Socket creation failed" << endl;
		exit(EXIT_FAILURE);
	}

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = INADDR_ANY;
	sockAddr.sin_port = 8080 + workerId;

	int bindStatus = bind(sockfd, (struct sockaddr*) &sockAddr, sizeof(sockAddr));
	if (bindStatus < 0) {
		cout << "Binding failed" << endl;
		exit(EXIT_FAILURE);
	}

	this->nodes = map<int, vector<int>>();

	this->workersSockAddr = vector<sockaddr_in>();
}

void Worker::setWorkersSockAddr() {
	sockaddr_in addrTmp;
	addrTmp.sin_family = AF_INET;

	// Create list of workers ip and port
	for (int i = 0; i < numWorkers; ++i) {
		addrTmp.sin_port = 8080 + i;

		if (inet_pton(AF_INET, "127.0.0.1", &addrTmp.sin_addr) <= 0)
		{
			cout << "Invalid address for " << i << " worker" << endl;
			exit(EXIT_FAILURE);
		}

		workersSockAddr.push_back(addrTmp);
	}
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
			nodeNeighbors.push_back(std::stoi(tmp)); // save node neighbors to vector
		}

		nodes.insert(std::pair<int, std::vector<int>>(node, nodeNeighbors));
		nodeNeighbors.clear();
	}
}

void Worker::broadcastNodeInfo() {
	string message = "Hi from me";
	char buffer[1024] = {0};
	int sock = 0;

	for (int i = 0; i < numWorkers; ++i) {
		if (i != id) {
			sock = socket(AF_INET, SOCK_STREAM, 0);
			connect(sock, (struct sockaddr*)&workersSockAddr[i], sizeof(workersSockAddr[i]));
			send(sock, &message[0], message.length(), 0);
			read(sock, buffer, 1024);
			printf("%s\n", buffer);
			close(sock);
		}
	}
}
