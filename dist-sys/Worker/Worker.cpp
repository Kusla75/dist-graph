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
	this->clusteringCoeff = map<int, float>();
	this->otherWorkersNodes = map<int, vector<int>>();

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
	string line;
	int node = 0;
	vector<int> nodeNeighbors;

	path = path + "/" + to_string(id) + ".txt";
	ifstream f(path);

	while (getline(f, line)) {
		string str1 = line.substr(0, line.find(",")); // str1 holds node
		string str2 = line.substr(line.find(",") + 2); // str2 holds neighbors of node
		str2.erase(remove(str2.begin(), str2.end(), '\r'), str2.end()); // remove /r from string

		node = stoi(str1);

		stringstream ss(str2);
		string tmp;

		while (getline(ss, tmp, ' '))
		{
			nodeNeighbors.push_back(stoi(tmp)); // save node neighbors to vector
		}

		sort(nodeNeighbors.begin(), nodeNeighbors.end());
		nodes.insert(pair<int, vector<int>>(node, nodeNeighbors));
		nodeNeighbors.clear();
	}
}

void Worker::sendDataToWorker(Worker w, int workerId, int* data, int dataLen) {
	int val = -1;
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sockAddr = w.getWorkersSockAddr()[workerId];
	while (val < 0) {
		val = connect(sock, (sockaddr*) &sockAddr, sizeof(sockAddr));
	}
	send(sock, data, dataLen, 0);
	close(sock);
}

void Worker::broadcastNodeInfo(Worker w) {

	// buffer stores id of worker and nodes that it has
	int len = (w.getNodes().size() + 1)*sizeof(int);
	int buffer[len];
	buffer[0] = w.getId();

	int i = 1;
	for (const auto& nodesPair : w.getNodes()){
		buffer[i++] = nodesPair.first;
	}

	vector<thread> threads;
	for (i = 0; i < w.getNumWorkers(); ++i) {
		if (i != w.getId()) {
			threads.push_back(thread(Worker::sendDataToWorker, ref(w), i, &buffer[0], len));
		}
	}
	for (i = 0; i < w.getNumWorkers()-1; ++i) {
		threads[i].join();
	}
}

void Worker::recvWorkersNodeInfo(Worker& w) {
	int buffer[SIZE];
	sockaddr_in address = w.getSockAddr();
	int addrLen = sizeof(w.getSockAddr());
	int sock;

	listen(w.getSockfd(), 5);
	for (int n = 0; n < w.getNumWorkers() - 1; ++n)
	{
		sock = accept(w.getSockfd(), (sockaddr*)&address, (socklen_t*)&addrLen);
		int byteSize = recv(sock, buffer, SIZE, 0);
		int len = byteSize / sizeof(int);

		vector<int> tempVec(buffer + 1, buffer + len);
		sort(tempVec.begin(), tempVec.end());
		w.getOtherWorkersNodes().insert(pair<int, vector<int>>(buffer[0], tempVec));

		close(sock);
	}
}

vector<int> Worker::requestNodeNeighbors(Worker w, int node) {
	int workerId = 0;

	// Searches which worker to contanct to request data
	for (auto pair : w.getOtherWorkersNodes()) {
		if (binary_search(pair.second.begin(), pair.second.end(), node)) {
			workerId = pair.first;
			break;
		}
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sockAddr = w.getWorkersSockAddr()[workerId];
	connect(sock, (sockaddr*)&sockAddr, sizeof(sockAddr));

	int buffer[SIZE];
	send(sock, &node, sizeof(node), 0);
	int byteSize = recv(sock, buffer, SIZE, 0);
	int len = byteSize / sizeof(int);
	close(sock);

	vector<int> nodeNeighbors(buffer, buffer + len);
	return nodeNeighbors;
}

void Worker::recvNodeNeighborsRequest(Worker w) {
	int buffer[SIZE];
	sockaddr_in address = w.getSockAddr();
	int addrLen = sizeof(w.getSockAddr());
	int sock;
	vector<int> tempVec;

	while (true) {
		listen(w.getSockfd(), 5);
		sock = accept(w.getSockfd(), (sockaddr*)&address, (socklen_t*)&addrLen);
		recv(sock, buffer, SIZE, 0);

		tempVec = w.getNodes()[buffer[0]];
		send(sock, tempVec.data(), tempVec.size()*sizeof(int), 0);
		close(sock);
	}
}

void Worker::calculateClusteringCoeff(Worker& w) {
	int node;
	int neighbor;
	int neighborsEdges = 0;
	int numNeighbors = 0;
	float coeff = 0;
	int totalneighborEdges = 0;
	vector<int> neighborNeighbors;

	for (auto pr : w.getNodes()) {
		node = pr.first;
		numNeighbors = pr.second.size();

		neighborsEdges = 0;
		totalneighborEdges = (numNeighbors * (numNeighbors - 1)) / 2;

		for (int i = 0; i < numNeighbors; ++i) {

			neighbor = pr.second[i];

			// Check if worker has node in memory
			if (w.getNodes().find(neighbor) == w.getNodes().end()) {
				// not found
				neighborNeighbors = requestNodeNeighbors(w, neighbor);

				for (int j = i + 1; j < numNeighbors; ++j) {

					if (binary_search(neighborNeighbors.begin(), neighborNeighbors.end(), pr.second[j])) {
						neighborsEdges++;
					}
				}
			}
			else {
				// found
				for (int j = i + 1; j < numNeighbors; ++j) {

					if (binary_search(w.getNodes()[neighbor].begin(), w.getNodes()[neighbor].end(), pr.second[j])) {
						neighborsEdges++;
					}
				}
			}
		}
		
		coeff = (float)neighborsEdges / (float)totalneighborEdges;
		w.getClusteringCoeff().insert(pair<int, float>(node, coeff));
	}
}