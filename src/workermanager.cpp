#include "workermanager.h"
#include "protoutils.hpp"
#include <spdlog/spdlog.h>

WorkerManager::WorkerManager(){
    timeout_thread = std::thread(&WorkerManager::checkConnections, this);
    timeout_thread.detach();
}


WorkerManager::~WorkerManager()
{
    timeout_thread.join();
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
    std::lock_guard<std::mutex> lock(mtx);
    spdlog::info("Worker {} sign off", worker->id);
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
    int available = availableWorkes.size();
    if(job.status == JobStatus::job_new){
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


void WorkerManager::splitRawData(std::string rawData, std::vector<std::string> &data,
                            int workes, bool cropWords){
    int size = rawData.size();
    int chunk = size / workes;
    int chunk_counter = 0;
    for(int j=1; j<size; j++){
        if(chunk_counter >= chunk && (rawData[j] == ' ' || cropWords)){
            std::string dataChunk = rawData.substr(0, chunk_counter);
            data.push_back(dataChunk);
            rawData = rawData.substr(chunk_counter);
            chunk_counter = 0;
        }
        chunk_counter++;
    }
    data.push_back(rawData);
}


void WorkerManager::assignMapping(Job job, std::set<connection_ptr> &availableWorkes){
    std::vector<std::string> data;
    splitRawData(job.data, data, availableWorkes.size(), true);
    for(auto &worker : availableWorkes){
        worker->is_available = false;
        mapreduce::TaskMap task = MessageGenerator::TaskMap(job.type, data.back());
        data.pop_back();
        worker->sendMessage(task);
        spdlog::info("Job {} assigned to worker {}", job.id, worker->id);
    }
}


void WorkerManager::checkConnections(){
    spdlog::info("Checking connections");
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> lock(mtx);
        for(auto &worker : workers){
            if(worker->isConnected()){
                if(worker->last_active + std::chrono::seconds(25) < std::chrono::system_clock::now()){
                    spdlog::error("Worker {} unreachable", worker->id);
                }else if(worker->last_active + std::chrono::seconds(15) < std::chrono::system_clock::now()){
                    spdlog::info("Worker {} timeout", worker->id);
                    mapreduce::SignOff signOff = MessageGenerator::SignOff(0, mapreduce::ConnectionType::WORKER);
                    worker->sendMessage(signOff);
                    worker->closeConnection();
                }else if(worker->last_active + std::chrono::seconds(10) < std::chrono::system_clock::now()){
                    mapreduce::Ping ping = MessageGenerator::Ping();
                    worker->sendMessage(ping);
                }
            }
        }
    }
}