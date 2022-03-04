#ifndef WORKMANAGER_H
#define WORKMANAGER_H

#include <map>
#include "job.hpp"
#include "pipe.hpp"

struct WorkerObject {
    int id{-1};
    bool is_available;

    WorkerObject(){};
    WorkerObject(int id) 
      : id(id), is_available(true) {};
};

class WorkerManager{
    std::map<int, WorkerObject> workers;
    asio::ip::port_type port{1500};

    int generateWorkerId();

    public:
        WorkerManager();
        ~WorkerManager();

        void operator()();
        bool assignJob(Job job);
};

#endif