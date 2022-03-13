#include "workermanager.h"
#include <spdlog/spdlog.h>

WorkerManager::WorkerManager(){}


WorkerManager::~WorkerManager()
{
}

void WorkerManager::join(connection_ptr worker)
{
    totalConnections++;
    spdlog::info("Worker {} connected", worker->id);
    std::lock_guard<std::mutex> lock(mtx);
    workers.insert(worker);
}


void WorkerManager::leave(connection_ptr worker)
{
    totalConnections--;
    spdlog::info("Worker {} sign off", worker->id);
    std::lock_guard<std::mutex> lock(mtx);
    workers.erase(worker);
}


int WorkerManager::generateID(){
    return totalConnections;
}