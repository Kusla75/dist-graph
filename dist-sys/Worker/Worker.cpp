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

void Worker::addToOtherWorkersNodes(pair<int, vector<int>> p) {
	otherWorkersNodes.insert(p);
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
	int buffer[BUFFERSIZE];
	sockaddr_in address = w.getSockAddr();
	int addrLen = sizeof(w.getSockAddr());
	int sock;

	listen(w.getSockfd(), 5);
	for (int n = 0; n < w.getNumWorkers() - 1; ++n)
	{
		sock = accept(w.getSockfd(), (sockaddr*)&address, (socklen_t*)&addrLen);
		int byteSize = recv(sock, buffer, BUFFERSIZE, 0);
		int len = byteSize / sizeof(int);

		vector<int> tempVec(buffer + 1, buffer + len);
		w.addToOtherWorkersNodes(pair<int, vector<int>>(buffer[0], tempVec));

		close(sock);
	}
}