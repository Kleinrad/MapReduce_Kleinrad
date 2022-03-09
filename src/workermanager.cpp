#include "workermanager.h"
#include <spdlog/spdlog.h>

WorkerManager::WorkerManager(){}


WorkerManager::~WorkerManager()
{
}


void WorkerManager::acceptWorker(){
}


void WorkerManager::join(worker_ptr worker)
{
    std::lock_guard<std::mutex> lock(mtx);
    totalWorkerConnections++;
    workers.insert(worker);
}


void WorkerManager::leave(worker_ptr worker)
{
    std::lock_guard<std::mutex> lock(mtx);
    workers.erase(worker);
}


int WorkerManager::generateWorkerId(){
    return totalWorkerConnections;
}