#include "workermanager.h"
#include <spdlog/spdlog.h>

WorkerManager::WorkerManager(){}


WorkerManager::~WorkerManager()
{
}

void WorkerManager::join(connection_ptr worker)
{
    std::lock_guard<std::mutex> lock(mtx);
    workers.insert(worker);
}


void WorkerManager::leave(connection_ptr worker)
{
    std::lock_guard<std::mutex> lock(mtx);
    workers.erase(worker);
}