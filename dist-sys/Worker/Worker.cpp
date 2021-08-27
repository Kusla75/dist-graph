#include "Worker.h"

Worker::Worker(int workerId, int numWorkers) {
	this->id = workerId;
	this->numWorkers = numWorkers;
	this->sockfd = -1;

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = INADDR_ANY;
	sockAddr.sin_port = FIX_PORT + workerId;

	this->nodes = map<int, vector<int>>();
	this->clusteringCoeff = map<int, double>();
	this->otherWorkersNodes = map<int, vector<int>>();

	this->workersSockAddr = vector<sockaddr_in>();
	this->workConsensus = vector<bool>(this->numWorkers, false);
}

void Worker::createAndBindSock(int type) {
	close(sockfd);

	sockfd = socket(AF_INET, type, 0);
	if (sockfd == 0) {
		cout << "Socket creation failed" << endl;
		exit(EXIT_FAILURE);
	}

	int bindStatus = bind(sockfd, (sockaddr*)&sockAddr, sizeof(sockAddr));
	if (bindStatus < 0) {
		cout << "Binding failed" << endl;
		exit(EXIT_FAILURE);
	}
}

void Worker::setWorkersSockAddr() {
	sockaddr_in addrTmp;
	addrTmp.sin_family = AF_INET;

	// Create list of workers ip and port
	for (int i = 0; i < numWorkers; ++i) {
		addrTmp.sin_port = FIX_PORT + i;

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

// ----------------------------------------------------------------------------------------------------------------

void Worker::broadcastNodeInfo(Worker w) {

	// buffer stores nodes that it has
	int len = w.getNodes().size() + 1;
	int buffer[len];
	buffer[0] = w.getId(); // first int in buffer represents worker id that is sending data

	// read all keys (nodes) from map and store into buffer
	int i = 1;
	for (auto nodesPair : w.getNodes()){
		buffer[i++] = nodesPair.first;
	}

	vector<thread> threads;
	for (i = 0; i < w.getNumWorkers(); ++i) {
		if (i != w.getId()) {
			threads.push_back(thread(Worker::sendNodeInfoToWorker, ref(w), i, &buffer[0], len*sizeof(int)));
		}
	}
	for (i = 0; i < w.getNumWorkers()-1; ++i) {
		threads[i].join();
	}
}

void Worker::recvNodeInfo(Worker& w) {
	int buffer[SIZE];
	sockaddr_in addr;
	socklen_t addrLen = sizeof(sockaddr_in);
	int sock;

	listen(w.getSockfd(), 5);
	for (int n = 0; n < w.getNumWorkers() - 1; ++n)
	{
		sock = accept(w.getSockfd(), (sockaddr*)&addr, &addrLen);
		int byteSize = recv(sock, buffer, SIZE, 0);
		int len = byteSize / sizeof(int);

		vector<int> tempVec(buffer + 1, buffer + len);
		sort(tempVec.begin(), tempVec.end());

		w.getOtherWorkersNodes().insert(pair<int, vector<int>>(buffer[0], tempVec));

		close(sock);
	}
}

void Worker::sendNodeInfoToWorker(Worker w, int workerId, int* data, int dataLen) {
	int val = -1;
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sockAddr = w.getWorkersSockAddr()[workerId];
	while (val < 0) {
		val = connect(sock, (sockaddr*)&sockAddr, sizeof(sockAddr));
	}
	send(sock, data, dataLen, 0);
	close(sock);
}

// ----------------------------------------------------------------------------------------------------------------

vector<int> Worker::requestNodeNeighbors(Worker w, int node) {
	int workerId = 0;

	// Searches which worker to contanct to request data
	for (auto pair : w.getOtherWorkersNodes()) {
		if (binary_search(pair.second.begin(), pair.second.end(), node)) {
			workerId = pair.first;
			break;
		}
	}

	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in addr = w.getWorkersSockAddr()[workerId];
	socklen_t addrLen = sizeof(addr);

	int buffer[SIZE];
	buffer[0] = NEIGHREQ;
	buffer[1] = node;

	sendto(sock, (int*) buffer, sizeof(int)*2, 0, (sockaddr*) &addr, addrLen);
	int byteSize = recvfrom(sock, buffer, SIZE, 0, (sockaddr*) &addr, &addrLen);
	int len = byteSize / sizeof(int);
	close(sock);

	vector<int> nodeNeighbors(buffer, buffer + len);
	return nodeNeighbors;
}

void Worker::listenForRequest(Worker& w) {
	int buffer[SIZE];
	sockaddr_in addr;
	socklen_t addrLen = sizeof(addr);
	vector<int> tempVec;
	bool quit = false;

	while (!quit) {
		int byteSize = recvfrom(w.getSockfd(), buffer, SIZE, 0, (sockaddr*) &addr, &addrLen);
		int len = byteSize / sizeof(int);

		switch (buffer[0]) {
			case NEIGHREQ: {
				tempVec = w.getNodes()[buffer[1]];
				sendto(w.getSockfd(), tempVec.data(), tempVec.size() * sizeof(int), 0, (sockaddr*)&addr, addrLen);
			}; break;
			case CONS: {
				w.getWorkConsensus()[buffer[1]] = true;
				quit = checkWorkConsensus(w);
			}; break;
		}
	}
}

void Worker::calculateClusteringCoeff(Worker& w) {
	int node;
	int neighbor;
	int neighborsEdges = 0;
	int numNeighbors = 0;
	double coeff = 0;
	int totalneighborEdges = 0;
	vector<int> neighborNeighbors;

	for (auto pr : w.getNodes()) {
		node = pr.first;
		numNeighbors = pr.second.size();

		if (numNeighbors == 1) {
			coeff = 0;
		}
		else {
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
			coeff = (double)neighborsEdges / (double)totalneighborEdges;
		}

		w.getClusteringCoeff().insert(pair<int, double>(node, coeff));
	}
}

bool Worker::broadcastWorkConsensus(Worker& w) {
	int i;
	vector<thread> threads;

	w.getWorkConsensus()[w.getId()] = true;
	int buffer[2];
	buffer[0] = CONS;
	buffer[1] = w.getId();

	for (i = 0; i < w.getNumWorkers(); ++i) {
		if (i != w.getId()) {
			threads.push_back(thread(Worker::sendDataToWorker, w, i, &buffer[0], sizeof(int) * 2));
		}
	}
	for (i = 0; i < w.getNumWorkers() - 1; ++i) {
		threads[i].join();
	}

	return checkWorkConsensus(w);
}

void Worker::sendDataToWorker(Worker w, int workerId, int* data, int dataLen) {
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in addr = w.getWorkersSockAddr()[workerId];
	socklen_t addrLen = sizeof(addr);
	sendto(sock, data, dataLen, 0, (sockaddr*) &addr, addrLen);

	close(sock);
}

bool Worker::checkWorkConsensus(Worker w) {

	for (int i = 0; i < w.getNumWorkers(); ++i) {
		if (!w.getWorkConsensus()[i]) {
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------------------------------------------------

void Worker::LogResults(Worker w, string path, chrono::steady_clock::time_point startTime) {

	auto endTime = chrono::steady_clock::now();
	int executionTime = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
	cout << "Worker " << w.getId() << " time: " << executionTime << " ms" << endl;

	ofstream file;
	string fileName = path + to_string(w.getId()) + "_res.txt";

	string command = "mkdir -p " + path;
	system(command.c_str());

	file.open(fileName);
	for (auto pair : w.getClusteringCoeff()) {
		file << to_string(pair.first) << ": " << to_string(pair.second) << endl;
	}
	file.close();

	fileName = path + to_string(w.getId()) + "_info.txt";
	file.open(fileName);
	file << "Execution time: " << executionTime << " ms" << endl;

	file.close();
}