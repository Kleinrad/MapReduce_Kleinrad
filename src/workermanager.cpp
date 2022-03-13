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
    spdlog::debug("JOB assigned {}, status {}", job.id, job.status);
    spdlog::debug("Workers connected {}", workers.size());
    for(auto &worker : workers){
        spdlog::debug("Worker {} is available {}", worker->id, worker->is_available);
        if(worker->is_available){
            availableWorkes.insert(worker);
        }
    }
    int available = availableWorkes.size();
    if(job.status == JobStatus::job_new){
        spdlog::debug("JOB {} has {} available workers", job.id, available);
        if((job.mappers == -1 && available > 0) 
        || available >= job.mappers){
            job.status = JobStatus::job_mapping;
            assignMapping(job, availableWorkes);
        }else{
            job.status = JobStatus::job_queued;
            spdlog::info("Job {} is queued: not enough workes availible", job.id);
            jobs.push(job);
            return false;
        }
    }if(job.status == JobStatus::job_queued){
        if((job.mappers == -1 && available > 0) 
        || available >= job.mappers){
            
        }else{
            job.status = JobStatus::job_queued;
            spdlog::info("Job {} is queued: not enough workes availible", job.id);
            jobs.push(job);
            return false;
        }
    }if(job.status == JobStatus::job_mapped){
        if((job.reducers == -1 && available > 0) 
        || available >= job.reducers){
            
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


void WorkerManager::splitRawData(std::string *data, int workes, bool cropWords){
    int size = data[0].size();
    int chunk = size / workes;
    int i = 0;
    int chunk_counter = 0;
    std::string rawData = data[0];
    data = new std::string[workes];
    for(int j=0; i<size; j++){
        if(chunk_counter >= chunk && (rawData[j] == ' ' || cropWords)){
            chunk_counter = 0;
            data[i] = rawData.substr(0, j);
            rawData = rawData.substr(j+1);
            i++;
        }
        chunk_counter++;
    }
}


void WorkerManager::assignMapping(Job job, std::set<connection_ptr> &availableWorkes){
    std::string data[1];
    data[0] = job.data;
    splitRawData(data, availableWorkes.size(), false);
    for(auto d : data){
        spdlog::debug("Mapping data {}", d);
    }
    /*for(auto &worker : availableWorkes){
        worker->is_available = false;
        mapreduce::TaskMap task;
        worker->sendMessage(task);
        spdlog::info("Job {} assigned to worker {}", job.id, worker->id);
    }*/
}

