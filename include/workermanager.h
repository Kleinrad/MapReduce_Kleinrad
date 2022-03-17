#ifndef WORKMANAGER_H
#define WORKMANAGER_H

#include <set>
#include <queue>
#include <map>
#include <vector>
#include <thread>
#include "pipe.hpp"
#include "job.hpp"
#include "connectionobject.hpp"

class WorkerManager{
    std::set<connection_ptr> workers;
    std::queue<Job> jobs;
    std::map<int, ActiveJob> activeJobs;
    asio::ip::port_type port{1500};
    std::thread timeout_thread;
    std::mutex workerMtx;
    std::mutex activeJobMtx;
    std::mutex jobsMtx;
    int totalConnections{0};

    void splitRawData(std::string rawData, std::vector<std::string> &data
        , int workes, bool cropWords);
    void assignMapping(Job job
        , std::set<connection_ptr> &availableWorkes);
    void assignReduce(Job job
        , std::set<connection_ptr> &availableWorkes);
    void reAssignTask(int worker_id);
    void checkConnections();

    public:
        WorkerManager();
        ~WorkerManager();

        void join(connection_ptr worker);
        void leave(connection_ptr worker);
        bool assignJob(Job job);
        int generateID();
};

#endif