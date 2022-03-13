#ifndef WORKMANAGER_H
#define WORKMANAGER_H

#include <set>
#include <queue>
#include <map>
#include <thread>
#include "pipe.hpp"
#include "job.hpp"
#include "connectionobject.hpp"

class WorkerManager{
    std::set<connection_ptr> workers;
    std::queue<Job> jobs;
    asio::ip::port_type port{1500};
    std::mutex mtx;
    int totalConnections{0};

    void assignMapping(Job job
        , std::set<connection_ptr> &availableWorkes);
    void assignReduce(Job job
        , std::set<connection_ptr> &availableWorkes);

    public:
        WorkerManager();
        ~WorkerManager();

        void join(connection_ptr worker);
        void leave(connection_ptr worker);
        bool assignJob(Job job);
        int generateID();
};

#endif