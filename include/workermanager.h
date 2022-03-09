#ifndef WORKMANAGER_H
#define WORKMANAGER_H

#include <set>
#include <thread>
#include "pipe.hpp"
#include "job.hpp"

class WorkerObject {
    protected:
        int id{-1};
        bool is_available;

    public:
        WorkerObject(int id) 
        : id(id), is_available(true) {};

};

typedef std::shared_ptr<WorkerObject> worker_ptr;

class WorkerManager{
    std::set<worker_ptr> workers;
    asio::ip::port_type port{1500};
    std::mutex mtx;
    int totalWorkerConnections{0};

    public:
        WorkerManager();
        ~WorkerManager();

        int generateWorkerId();
        void acceptWorker();
        void join(worker_ptr worker);
        void leave(worker_ptr worker);
        bool assignJob(Job job);
};

#endif