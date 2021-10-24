#include "Worker.h"

mutex Worker::mtx;

Worker::Worker(int workerId, int numWorkers) {
	this->id = workerId;
	this->numWorkers = numWorkers;
	this->sockfd = -1;

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = INADDR_ANY;
	sockAddr.sin_port = FIX_PORT + workerId;

	this->nodes = map<int, vector<int>>();
	this->clusteringCoeff = map<int, float>();
	this->otherWorkersNodes = map<int, vector<int>>();

	this->workersSockAddr = vector<sockaddr_in>();
	this->workConsensus = vector<bool>(this->numWorkers, false);
	this->workersStatus = vector<Status>(this->numWorkers, ACTIVE);
	this->timeCheckpoints.push_back(0);
	this->numMessages = 0;
	this->stat = ACTIVE;
}

void Worker::createAndBindSock(int type) {
	close(sockfd);

	sockfd = socket(AF_INET, type, 0);
	if (sockfd == 0) {
		cout << "ID: " << id <<" - socket creation failed" << endl;
		exit(EXIT_FAILURE);
	}

	int bindStatus = ::bind(sockfd, (sockaddr*)&sockAddr, sizeof(sockAddr));
	if (bindStatus < 0) {
		cout << "ID: " << id << " - binding failed" << endl;
		exit(EXIT_FAILURE);
	}
}

void Worker::setWorkersSockAddr(string ipFileName) {
	sockaddr_in addrTmp;
	addrTmp.sin_family = AF_INET;

	int i;
	ifstream ipFile(ipFileName.c_str());
	vector<string> ipAddr;
	string line;

	if (ipFile.good()) {
		while (getline(ipFile, line)) {
			line.erase(remove(line.begin(), line.end(), '\r'), line.end());
			ipAddr.push_back(line);
		}
	}
	else {
		for (i = 0; i < numWorkers; ++i) {
			ipAddr.push_back("127.0.0.1");
		}
	}

	// Create list of workers ip and port
	for (i = 0; i < numWorkers; ++i) {
		addrTmp.sin_port = FIX_PORT + i;

		if (inet_pton(AF_INET, ipAddr[i].c_str(), &addrTmp.sin_addr) <= 0)
		{
			cout << "ID: " << id << " - invalid address for " << i << " worker" << endl;
			exit(EXIT_FAILURE);
		}

		workersSockAddr.push_back(addrTmp);
	}
}

void Worker::loadNodesData(string path) {
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

		clusteringCoeff.insert(pair<int, float>(node, -1));
	}
}

void Worker::setSockOpt(int sock, int sec, int microsec) {
	struct timeval tv;
	tv.tv_sec = sec;
	tv.tv_usec = microsec;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		cout << "ID: " << id << " - socket option timeout error " << errno << endl;
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
	for (i = 0; i < threads.size(); ++i) {
		threads[i].join();
		w.incNumMessages();
	}
}

void Worker::recvNodeInfo(Worker& w) {
	int buffer[SIZE/sizeof(int)];
	sockaddr_in addr;
	socklen_t addrLen = sizeof(sockaddr_in);
	vector<int>* tempVec;
	int sock;

	listen(w.getSockfd(), 5);
	for (int n = 0; n < w.getNumWorkers() - 1; ++n)
	{
		sock = accept(w.getSockfd(), (sockaddr*)&addr, &addrLen);
		int byteSize = recv(sock, buffer, SIZE, 0);
		int len = byteSize / sizeof(int);

		tempVec = new vector<int>(buffer + 1, buffer + len);
		sort(tempVec->begin(), tempVec->end());

		w.getOtherWorkersNodes().insert(pair<int, vector<int>>(buffer[0], *tempVec));
		delete tempVec;

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

vector<int> Worker::requestNodeNeighbors(Worker& w, int node) {
	int workerId = 0;
	vector<int> workersVec, nodeNeighbors;
	bool faultDetection = false;

	// Searches which workers to contanct to request data
	for (auto pair : w.getOtherWorkersNodes()) {
		workerId = pair.first;

		if (binary_search(pair.second.begin(), pair.second.end(), node) && w.getWorkersStatus()[workerId] == ACTIVE) {
			workersVec.push_back(workerId);
		}
	}

	int buffer[SIZE/sizeof(int)];
	int byteSize, len = 0;

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	//w.setSockOpt(sock, 10, 0);

	for (int workerId : workersVec) {
		sockaddr_in addr = w.getWorkersSockAddr()[workerId];
		socklen_t addrLen = sizeof(addr);

		buffer[0] = NEIGHREQ;
		buffer[1] = node;
		sendto(sock, (int*)buffer, sizeof(int) * 2, 0, (sockaddr*)&addr, addrLen);

		Worker::mtx.lock(); w.incNumMessages(); Worker::mtx.unlock();

		byteSize = recvfrom(sock, buffer, SIZE, 0, (sockaddr*)&addr, &addrLen);
		
		if (byteSize > 0) {
			len = byteSize / sizeof(int);
			nodeNeighbors = vector<int>(buffer, buffer + len);
			break;
		}
		else {
			faultDetection = true;
			w.getWorkersStatus()[workerId] = CRASH;
			cout << "ID: " << w.getId() << " - couldn't reach worker " << workerId << "" << endl;
		}
	}

	close(sock);

	if (faultDetection) {
		broadcastWorkersStatus(w);
	}

	return nodeNeighbors;
}

void Worker::listenForRequest(Worker& w) {
	int buffer[SIZE/sizeof(int)];
	sockaddr_in addr;
	socklen_t addrLen = sizeof(addr);
	vector<int> tempVec;
	bool quit = false;
	w.setSockOpt(w.getSockfd(), 0, 250000);

	while (!quit && w.getStatus() != CRASH) {

		int byteSize = recvfrom(w.getSockfd(), buffer, SIZE, 0, (sockaddr*) &addr, &addrLen);
		if (byteSize > 0) {
			switch (buffer[0]) {

				case NEIGHREQ: {
					tempVec = w.getNodes()[buffer[1]];
					sendto(w.getSockfd(), tempVec.data(), tempVec.size() * sizeof(int), 0, (sockaddr*)&addr, addrLen);

					Worker::mtx.lock(); w.incNumMessages(); Worker::mtx.unlock();
				}; break;

				case CONS: {
					w.getWorkConsensus()[buffer[1]] = true;
					quit = checkWorkConsensus(w);
				}; break;

				case CALCNODE: {
					float clusteringCoeff;
					memcpy(&clusteringCoeff, buffer + 2, sizeof(float));

					w.getClusteringCoeff()[buffer[1]] = clusteringCoeff;

				}; break;

				case STATUS: {
					for (int i = 0; i < w.getWorkersStatus().size(); ++i) {
						w.getWorkersStatus()[i] = static_cast<Status>(buffer[i+1]);
					}
				}; break;

			}
		}
		else {
			quit = checkWorkConsensus(w);
		}
		
	}

	close(w.getSockfd());
}

int Worker::calculateClusteringCoeff(Worker& w, int faultCounter) {
	int node;
	int neighbor;
	int neighborsEdges = 0;
	int numNeighbors = 0;
	float coeff = 0;
	int totalneighborEdges = 0;
	vector<int> neighborNeighbors;
	int counter = 0;

	for (auto pr : w.getNodes()) {
		node = pr.first;
		numNeighbors = pr.second.size();

		if (w.getClusteringCoeff()[node] == -1)
		{
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

				coeff = (float)neighborsEdges / (float)totalneighborEdges;
			}

			w.getClusteringCoeff()[node] = coeff;
			broadcastClusteringCoeffInfo(w, node, coeff);

			if (faultCounter > 0) {
				counter++;
				if (counter >= faultCounter) {
					w.getStatus() = CRASH;
					return -1;
				}
			}

			//cout << "ID: " << w.getId() << " - " << node << ": " << coeff << endl; // Debug
		}
	}
}

void Worker::broadcastClusteringCoeffInfo(Worker& w, int node, float clusteringCoeff) {
	int i;
	vector<thread> threads;

	int buffer[3];
	buffer[0] = CALCNODE;
	buffer[1] = node;
	memcpy(buffer+2, &clusteringCoeff, sizeof(float));

	for (i = 0; i < w.getNumWorkers(); ++i) {
		if (i != w.getId() && w.getWorkersStatus()[i] == ACTIVE) {
			threads.push_back(thread(Worker::sendDataToWorker, w, i, &buffer[0], sizeof(int) * 3));
		}
	}
	for (i = 0; i < threads.size(); ++i) {
		threads[i].join();
		Worker::mtx.lock(); w.incNumMessages(); Worker::mtx.unlock();
	}
}

void Worker::broadcastWorkersStatus(Worker& w) {
	int i;
	vector<thread> threads;

	int size = w.getNumWorkers();
	int buffer[size+1];
	buffer[0] = STATUS;
	copy(w.getWorkersStatus().begin(), w.getWorkersStatus().end(), buffer+1);

	for (i = 0; i < w.getNumWorkers(); ++i) {
		if (i != w.getId() && w.getWorkersStatus()[i] == ACTIVE) {
			threads.push_back(thread(Worker::sendDataToWorker, w, i, &buffer[0], sizeof(int) * size));
		}
	}
	for (i = 0; i < threads.size(); ++i) {
		threads[i].join();
		Worker::mtx.lock(); w.incNumMessages(); Worker::mtx.unlock();
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
		if (i != w.getId() && w.getWorkersStatus()[i] == ACTIVE) {
			threads.push_back(thread(Worker::sendDataToWorker, w, i, &buffer[0], sizeof(int) * 2));
		}
	}
	for (i = 0; i < threads.size(); ++i) {
		threads[i].join();
		Worker::mtx.lock(); w.incNumMessages(); Worker::mtx.unlock();
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
		if (!w.getWorkConsensus()[i] && w.getWorkersStatus()[i] == ACTIVE) {
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------------------------------------------------

void Worker::addTimeCheckpoint(chrono::steady_clock::time_point& startTime){

	int timeSum = totalTime();

	auto endTime = chrono::steady_clock::now();
	int executionTime = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();

	timeCheckpoints.push_back(executionTime - timeSum);
}

int Worker::totalTime() {
	int totalTime = 0;
	for (int i = 0; i < timeCheckpoints.size(); ++i) {
		totalTime += timeCheckpoints[i];
	}
	return totalTime;
}

void Worker::incNumMessages(int val) {
	numMessages += val;
}

void Worker::logResults(Worker w, string path) {

	ofstream file;
	string fileName = path + to_string(w.getId()) + "_res.txt";

	string command = "mkdir -p " + path;
	system(command.c_str());

	file.open(fileName);
	for (auto pair : w.getClusteringCoeff()) {
		ostringstream stream;
		stream << setprecision(12) << pair.second;
		file << to_string(pair.first) << ": " << stream.str() << endl;
	}
	file.close();

	fileName = path + to_string(w.getId()) + "_info.txt";
	file.open(fileName);
	file << "Init time: " << w.getTimeCheckpoint()[1] << " ms" << endl;
	file << "Calculating time: " << w.getTimeCheckpoint()[2] << " ms" << endl;
	file << "Consensus time: " << w.getTimeCheckpoint()[3] << " ms" << endl;
	file << "Total time: " << w.totalTime() << " ms" << endl;
	file << "\nTotal number of messages: " << w.getNumMessages() << endl;

	file.close();

	cout << "ID: " << w.getId() << " - time and messages: " << w.totalTime() << " ms, " << w.getNumMessages() << " messages" << endl;
}