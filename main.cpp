
#include "kvserver.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace miteDB;

using std::cin;
using std::cout;
using std::endl;

int main(int argc, char **argv) {
    if (argc < 3) {
	cout << "<usage> " << argv[0] << " port threadCount" << endl;
	return 1;
    }

    unsigned short port = atoi(argv[1]);
    int threadCount = atoi(argv[2]);

    KVServer server;
    if (server.Start(port, threadCount) == -1) {
	cout << "main thread can not start server " << endl;
	return 2;
    }

    while (1) {
	string str;
	cin >> str;

	if (str == "s") {
	    server.ShutDown();
	    }
    }

    return 0;
}
