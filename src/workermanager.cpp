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


bool WorkerManager::assignJob(Job job)
{
    std::lock_guard<std::mutex> lock(mtx);
    std::set<connection_ptr> availableWorkes;
    for(auto &worker : workers){
        if(worker->is_available){
            availableWorkes.insert(worker);
        }
    }
    if(job.status == JobStatus::job_new){
        if((job.mappers == -1 && availableWorkes.size() > 0) 
        || availableWorkes.size() >= job.mappers){
            job.status = JobStatus::job_mapping;
        }else{
            job.status = JobStatus::job_queued;
            spdlog::info("Job {} is queued: not enough workes availible", job.id);
            jobs.push(job);
            return false;
        }
    }if(job.status == JobStatus::job_queued){
        if((job.mappers == -1 && availableWorkes.size() > 0) 
        || availableWorkes.size() >= job.mappers){
            
        }else{
            job.status = JobStatus::job_queued;
            spdlog::info("Job {} is queued: not enough workes availible", job.id);
            jobs.push(job);
            return false;
        }
    }if(job.status == JobStatus::job_mapped){
        if((job.reducers == -1 && availableWorkes.size() > 0) 
        || availableWorkes.size() >= job.reducers){
            
        }else{
            job.status = JobStatus::job_queued;
            spdlog::info("Job {} is queued: not enough workes availible", job.id);
            jobs.push(job);
            return false;
        }
    }
    return true;
}


int WorkerManager::generateID(){
    return totalConnections;
}