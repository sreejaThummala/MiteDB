#include "threadpool.h"
#include <unistd.h>
#include <string.h>

using std::cout;
using std::endl;

namespace miteDB {

/////////////////////////////////////////////////Thread

Thread::Thread() : running(false), target(NULL) {}

Thread::Thread(Runnable& t) : running(false), target(&t) {}

Thread::~Thread() {    
    int err = pthread_attr_destroy(&tattr);
    if (err != 0) 
	cout << tid << " thread attr destory failed." << endl;
}

void* __thread_start(void* thread) {
    if (thread == NULL) return (void*)NULL;

    ((Thread *)thread)->Run();
    return (void*)0;
}

int Thread::start() {
    int ret = 0, err;

    if ((err = pthread_attr_init(&tattr)) != 0) {
	cout << tid << " thread attribute init fialed" << endl;
	ret = -1;
    }else if ((err = pthread_create(&tid, &tattr, 
				    &__thread_start, (void *)this)) != 0) {
	cout << tid << " thread create error" << endl;
	ret = -1;
    }

    pthread_yield();
    return ret;
}

pthread_t Thread::getId() const { 
    return tid;
}

int Thread::run() {
    running = true;
    int ret = -1;
    if (target != NULL) {
        ret = target->Run();
    } else { ­
        ret = this->DoRun();
    }
    
    running = false;

    cout << tid << " thread running end" << endl;
    return ret;
}

int Thread::doRun() {
    return 0;
}

bool Thread::isRunning() const {
    return running;
}

int Thread::join() {
  
    int ret = 0, err = 0;

    if ((err = pthread_join(tid, NULL)) != 0) {
	cout << "join thread " << tid << " failed with code " << err << endl;
	ret = -1;
    } else if ((err = pthread_attr_destroy(&tattr)) != 0) {
	cout << tid << " thread attr destory failed with code " << err << endl;
	ret = -1;
    }

    return ret;
}

int Thread::cancel() {
    int ret = 0, err = 0;

    if ((err = pthread_cancel(tid)) != 0) {
	cout << "Thread::Cacnel pthread cancel fail:" << strerror(err) << endl;
	ret = -1;
    }

    return ret;
}

/////////////////////////////////////////////////ThreadPool

ThreadPool::ThreadPool() {}

ThreadPool::~ThreadPool() {
    
    for (unsigned int i = 0; i < threads.size(); i ++) {
	threads[i]->join();
	delete threads[i];
    }
    threads.clear();
}

int ThreadPool::start(int threadCount, Runnable& target) {
   
    int ret = 0;

    for (int i = 0; i < threadCount; i ++) {
	Thread* thread = new Thread(target);

	if (thread == NULL) {
	    cout << "thread pool start target " << &target 
		 << " failed with not enough memory.\n" << endl;
	    ret = -1;
	    break;
	} else if (thread->Start() == -1) {
	    cout << "thread pool start " << i << " th thread failed" << endl;
	    ret = -1;
	    break;
	} else {
	    threads.push_back(thread);
	}
    }

    return ret;
}

int ThreadPool::joinAll() {
    
    int ret = 0;

    ThreadVec::iterator it = threads.end();
    while (it != threads.begin()) {
	it --;
	Thread* t = *it;
	if (t->Join() == -1) {
	    ret = -1;
	    break;
	}
	
	delete t;
	it = threads.erase(it);
    }

    return ret;
}

int ThreadPool::cancelAll() {
    int ret = 0;
    ThreadVec::iterator it = threads.end();
    while (it != threads.begin()) {
	it --;
	Thread *t = *it;
	if (t->Cancel() == -1 || t->Join() == -1) {
	    ret = -1;
	    break;
	}

	delete t;
	it = threads.erase(it);
    }
    
    return ret;
}

///////////////////////////////////////////////WorkQueue

WorkQueue::WorkQueue() : shutdown(false) {
    int err;
    
    if ((err = pthread_mutex_init(&mutex, NULL)) != 0) {
	cout << "WorkQueue mutex initialization failed with code " << err  << endl;
	return;
    } else if ((err = pthread_cond_init(&cond, NULL)) != 0) {
	cout << "WorkQueue conditional variable initialization failed" << endl;
	return;
    }
    works.clear();
}

WorkQueue::~WorkQueue() {
    int err;
    if ((err = pthread_mutex_destroy(&mutex)) != 0) {
	cout << "WorkQueue mutex dealloc failed" << endl;
    } else if ((err = pthread_cond_destroy(&cond)) != 0) {
	cout << "WorkQUeue cond dealloc failed" << endl;
    }
   
    works.clear();
}

int WorkQueue::addWork(Work* work) {

    if (shutdown) 
	return -1;

    int err;
    err = pthread_mutex_lock(&mutex);
    if (err != 0) {
	cout << "WorkQueue Add Work get mutex lock failed" << endl;
	return -1;
    }

    works.push_back(work);

    err = pthread_mutex_unlock(&mutex);
    if (err != 0) {
	cout << "WorkQueue Add Work unlock mutex lock failed" << endl;
	return -1;
    }

    err = pthread_cond_signal(&cond);
    if (err != 0) {
	cout << "WorkQueue AddWork v operation cond var failed." << endl;
	return -1;
    }

    return 0;
}

Work* WorkQueue::getWork() {
    if (shutdown)
	return NULL;
    
    int err;
    err = pthread_mutex_lock(&mutex);
    if (err != 0){
	cout << "WorkQueue GetWork get mutex lock failed." << endl;
	return NULL;
    }
    
    while (works.size() == 0 && !shutdown) {
	err = pthread_cond_wait(&cond, &mutex);
	if (err != 0) {
	    cout << "WorkQueue GetWork p cond var failed." << endl; 
	    return NULL;
	}
    }
    
    Work* work = NULL;
    if (works.size() != 0 && !shutdown) {
	work = works.front();
	works.pop_front();
    }

    err = pthread_mutex_unlock(&mutex);
    if (err != 0) {
	cout << "WorkrQueue GetWork free mutex lock failed." << endl;
	return NULL;
    }

    return work;
}

int WorkQueue::shutdown() {
    
    int err;
    err = pthread_mutex_lock(&mutex);
    if (err != 0) {
	cout << "WorkQueue Shutdown get mutex lock failed." << endl; 
	return -1;
    }

    shutdown = true;

    err = pthread_mutex_unlock(&mutex);
    if (err != 0) {
	cout << "WorkQueue Shutdown free mutex lock failed." << endl;
	return -1;
    }
    
    err = pthread_cond_broadcast(&cond);
    if (err != 0) {
	cout << "WorkQueue Shutdown broadcast signal failed." << endl; 
	return -1;
    }
    
    return 0;
}

bool WorkQueue::isShutdown() {
    return shutdown;
}

/////////////////////////////////////////////////Worker

Worker::Worker(WorkQueue& queue) : workQueue(queue) {}

Worker::~Worker() {
    
    cout << "Worker::~Worker" << endl;
}

int Worker::run() {
    
    while (1) {
	Work* work = workQueue.getWork();
	if (work != NULL) {
	    work->doWork();
	    if (work->needDelete()) {
		delete work;
	    }
	}
	else if (workQueue.isShutdown())
	    break;
    }

    return 0;
}

/////////////////////////////////////////////////WorkerThreadPool

WorkerThreadPool::WorkerThreadPool(): worker(workQueue) {}

WorkerThreadPool::~WorkerThreadPool() {
    
}

int WorkerThreadPool::start(int threadCount) {
    return pool.start(threadCount, worker);
}

int WorkerThreadPool::start(int threadCount, Runnable& target) {
    return pool.start(threadCount, target);
}

int WorkerThreadPool::addWork(Work* work) {
    int ret = workQueue.addWork(work);
    return ret;
}

int WorkerThreadPool::shutdown() {
    return workQueue.shutdown();
}

int WorkerThreadPool::joinAll() {
    return pool.joinAll();
}

int WorkerThreadPool::cancelAll() {
    return pool.cancelAll();
}

} 



