
#ifndef MITEDB_THREAD_POOL_H
#define MITEDB_THREAD_POOL_H

#include <pthread.h>
#include <deque>
#include <vector>
#include <iostream>
#include "runnable.h"

namespace miteDB{
class Thread : public Runnable {
public:
    Thread();
    Thread(Runnable& t);
    virtual ~Thread();
    int start();
    pthread_t getId() const;
    int run();
    bool isRunning() const;
    int join();
    int cancel();
    
protected:
    virtual int doRun();
    pthread_t tid;
    pthread_attr_t tattr;
    bool running;
    Runnable* target;
};


class ThreadPool {
public:
    ThreadPool();
    virtual ~ThreadPool();
    virtual int start(int threadCount, Runnable& target);
    int joinAll();
    int cancelAll();

protected:
    typedef std::vector<Thread*> ThreadVec;
    ThreadVec threads;
};
class Work {
public:
    virtual ~Work() {}
    virtual bool needDelete() const = 0;
    virtual int doWork() = 0;
};

class WorkQueue {
public:
    WorkQueue();
    ~WorkQueue();
    int addWork(Work* work);
    Work* getWork();
    int shutdown();
    bool isShutdown();

private:
    typedef std::deque<Work*> Queue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool shutdown;
    Queue works;
};

class Worker : public Runnable {
public:
    Worker(WorkQueue& queue);
    ~Worker();
    virtual int run();
    
private:
    WorkQueue& workQueue;
};

class WorkerThreadPool {
public:
    WorkerThreadPool();
    ~WorkerThreadPool();
    int start(int threadCount);
    int start(int threadCount, Runnable& target);
    int addWork(Work* work);
    int shutdown();
    int joinAll();
    int cancelAll();

protected:

    WorkQueue workQueue;
    Worker worker;
    ThreadPool pool;

};

} 

#endif 
