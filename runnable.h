
#ifndef MITEDB_RUNNABLE_H
#define MITEDB_RUNNABLE_H

namespace miteDB {

class Runnable {
public:
    virtual ~Runnable() {}
    virtual int Run() = 0;
};

}  

#endif
