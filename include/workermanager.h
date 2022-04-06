/*
author: Kleinrad Fabian
matnr: i17053
file: workermanager.h
class: 5BHIF
catnr: 07
*/

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
    std::set<connectionPtr> workers;
    std::vector<Job> jobs;
    std::map<int, ActiveJob> activeJobs;
    asio::ip::port_type port{1500};
    std::thread timeout_thread;
    std::mutex workerMtx;
    std::mutex activeJobMtx;
    std::mutex jobsMtx;
    int totalConnections{0};

    void splitRawData(std::string rawData, std::vector<std::string> &data
        , int workes, bool cropWords);
    std::vector<std::vector<std::pair<std::string, int>>> shuffle
        (std::vector<std::pair<std::string, int>> &results, int workes);
    void assignMap(Job job
        , std::set<connectionPtr> &availableWorkes);
    void assignReduce(Job job
        , std::set<connectionPtr> &availableWorkes);
    void queueJob(Job job);
    void registerActiveJob(Job job);
    void checkConnections();

    public:
        WorkerManager();
        ~WorkerManager();

        void join(connectionPtr worker);
        void leave(connectionPtr worker);
        void mapResult(int job_id, int worker_id
            , std::vector<std::pair<std::string, int>> &result);
        bool reduceResult(int job_id, int worker_id
            , std::map<std::string, int> &result);
        bool assignJob(Job job);
        void reAssignTask(int worker_id);
        int generateID();
};

#endif