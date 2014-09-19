
#ifndef MM_TRAINING_PROCESS_POOL_H
#define MM_TRAINING_PROCESS_POOL_H

#include <string>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "runnable.h"

#define TASKMAXNUM 50

namespace miteDB {
class Process : Runnable{
public:
    Process();
    Process(Runnable& t);
    virtual ~Process();
    int start();
    int getId() const;
    int run();
    int kill();
    int wait();

protected:
    virtual int doRun();
    Runnable* target;
    pid_t pid;
};

class ProcessPool {
public:
    ProcessPool();
    ~ProcessPool();
    int start(int procCount, Runnable& target);
    int killAll();
    int waitAll();

protected:
    typedef std::vector<Process*> processVec;
    ProcessVec processes;
};

class Task {
public:
    virtual ~Task() {}
    virtual int doTask() = 0;
    virtual int toBuffer(void *toBuf) = 0;
    virtual int fromBuffer(void *fromBuf) = 0;
    virtual size_t getSize() = 0;
};

class TaskQueue {
public:
    TaskQueue(Task& taskType);
    ~TaskQueue();
    int addTask(Task& task);
    Task* getTask();
    int getSize();

protected:
    int semid;
    void pMutexSem();
    void vMutexSem();
    void pQueueSem();
    void vQueueSem();

    const int taskMaxNum;
    size_t taskSize;
    int shmid;
    void* shmptr;

    int getQueueSizeInShm();
    int setQueueSizeInShm(int size);

    int getQueueIndexInShm();
    int setQueueIndexInShm(int index);

    Task* getTaskInShm();
    int addTaskInShm(Task* task);
};
class Processor : public Runnable {
public:
    Processor(TaskQueue& queue, Task& tt);
    ~Processor();
    int run();
    
private:
    
    TaskQueue& taskQueue;
    Task& taskType;
};

class TaskProcessPool {
public:
    TaskProcessPool(Task& taskType);
    ~TaskProcessPool();
    int start(int processCount);
    int addTask(Task& task);
    int killAll();
    int waitAll();

private:

    TaskQueue taskQueue;
    Processor processor;
    ProcessPool pool;
};

} 

#endif
