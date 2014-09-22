
#ifndef MITEDB_KVSERVER_H
#define MITEDB_KVSERVER_H

#include "socket.h"
#include "threadpool.h"
#include "kvepoll.h"
#include "runnable.h"
#include "kvstore.h"

namespace miteDB {

class KVWork : public Work {
public:

    
    KVWork(int clientfd, KVStore *kvStore, KVEpoll *kvEpoll);
    virtual ~KVWork();
    virtual int DoWork();
    virtual bool NeedDelete() const;
private:

    void Get(char *buf, char *res);
    void Set(char *buf, char *res);
    void Delete(char *buf, char *res);
    void Stats(char *res);
    void Quit(char *res);
    void Prompt(char *res);

    ClientSocket *clientSocket;
    KVStore *kvStore;
    KVEpoll *kvEpoll;
    int epollfd;
    int clientfd;
};

void *__KVServerDaemon(void *arg);

class KVServer : private WorkerThreadPool {
public:

    KVServer();
    int Start(unsigned short port, int threadCount);
    int DaemonRun();
    void ShutDown();

private:

    unsigned short port;
    int threadCount;
    pthread_t daemonid;
    ServerSocket server;
    KVStore kvStore;
};

}

#endif
