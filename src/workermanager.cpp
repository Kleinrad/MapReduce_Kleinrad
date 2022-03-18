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
    std::lock_guard<std::mutex> lock(workerMtx);
    workers.insert(worker);
}


void WorkerManager::leave(connection_ptr worker)
{
    std::lock_guard<std::mutex> lock(workerMtx);
    spdlog::info("Worker {} sign off", worker->id);
    workers.erase(worker);
}


bool WorkerManager::assignJob(Job job)
{
    std::lock_guard<std::mutex> lock(workerMtx);
    std::set<connection_ptr> availableWorkes;
    for(auto &worker : workers){
        if(worker->is_available){
            availableWorkes.insert(worker);
        }
    }
    int available = availableWorkes.size();
    if(job.status == JobStatus::job_new || job.status == JobStatus::job_mapping){
        if((job.mappers == -1 && available > 0) 
        || (available >= job.mappers && available > 0)){
            job.status = JobStatus::job_mapping;
            registerActiveJob(job);       
            assignMapping(job, availableWorkes);    
        }else{
            job.status = JobStatus::job_queuedMap;
            queueJob(job);
            return false;
        }
    }if(job.status == JobStatus::job_queuedMap || job.status == JobStatus::job_queuedReduce){
        if((job.mappers == -1 && available > 0) 
        || (available >= job.mappers && available > 0)){
            if(job.status == JobStatus::job_queuedMap){
                job.status = JobStatus::job_mapping;
                registerActiveJob(job);
                assignMapping(job, availableWorkes);
            }else if(job.status == JobStatus::job_queuedReduce){
                job.status = JobStatus::job_reducing;
                //assignReducing(job, availableWorkes);
            }
        }else{
            queueJob(job);
            return false;
        }
    }if(job.status == JobStatus::job_mapped){
        if((job.reducers == -1 && available > 0) 
        || (available >= job.reducers && available > 0)){
            
        }else{
            job.status = JobStatus::job_queuedReduce;
            queueJob(job);
            return false;
        }
    }
    return true;
}


void WorkerManager::queueJob(Job job)
{
    spdlog::info("Job {} is queued: not enough workes available", job.id);
    std::lock_guard<std::mutex> lock(jobsMtx); 
    bool found = false;
    for(auto &j: jobs){
        if(j.id == job.id && j.status == job.status 
        && j.type == job.type){
            found = true;
            j.data += job.data;
            break;
        }
    }
    if(!found){
        jobs.push_back(job);
    }
}


int WorkerManager::generateID(){
    return totalConnections;
}


void WorkerManager::splitRawData(std::string rawData, std::vector<std::string> &data,
                            int workes, bool cropWords){
    if(workes != 0){
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
}


void WorkerManager::assignMapping(Job job, std::set<connection_ptr> &availableWorkes){
    std::vector<std::string> data;
    splitRawData(job.data, data, availableWorkes.size(), true);
    for(auto &worker : availableWorkes){
        worker->is_available = false;
        mapreduce::TaskMap task = 
            MessageGenerator::TaskMap(job.type, data.back(), job.id);
        std::lock_guard<std::mutex> lock(activeJobMtx);
        activeJobs[job.id].addWorker(worker->id, data.back());
        data.pop_back();
        worker->sendMessage(task);
        spdlog::info("Job {} assigned to worker {}", job.id, worker->id);
    }
}


void WorkerManager::mapResult(int job_id, int worker_id
            , std::set<std::pair<std::string, int>> &result){
    std::lock_guard<std::mutex> lock(activeJobMtx);
    activeJobs[job_id].addResults(result);
    activeJobs[job_id].removeWorker(worker_id);
    spdlog::info("Job {} worker {} finished [Job active {}]"
    , job_id, worker_id, activeJobs[job_id].isActive());
    if(!activeJobs[job_id].isActive()){
        spdlog::error("USE MAP RESULT");
        activeJobs.erase(job_id);
    }
}


void WorkerManager::reAssignTask(int worker_id){
    std::lock_guard<std::mutex> lock(activeJobMtx);
    spdlog::info("Reassigning task from worker {}", worker_id);
    for(auto &pair : activeJobs){
        if(pair.second.contains(worker_id)){
            Job job{pair.second, worker_id};
            assignJob(job);
            pair.second.removeWorker(worker_id);
        }    
    }
}


void WorkerManager::registerActiveJob(Job job){
    ActiveJob ajob{job};
    std::lock_guard<std::mutex> lock(activeJobMtx);
    spdlog::info("Job {} registered", job.id);
    activeJobs.insert({job.id, ajob}); 
}


void WorkerManager::checkConnections(){
    spdlog::info("Checking connections");
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> lock(workerMtx);
        bool has_available = false;
        for(auto &worker : workers){
            if(worker->isConnected()){
                if(worker->is_available)
                    has_available = true;
                if(worker->last_active + std::chrono::seconds(25) < std::chrono::system_clock::now()){
                    spdlog::error("Worker {} unreachable", worker->id);
                }else if(worker->last_active + std::chrono::seconds(12) < std::chrono::system_clock::now()){
                    spdlog::info("Worker {} timeout", worker->id);
                    mapreduce::SignOff signOff = MessageGenerator::SignOff(0, mapreduce::ConnectionType::WORKER);
                    worker->sendMessage(signOff);
                    worker->closeConnection();
                    if(!worker->is_available){
                        workerMtx.unlock();
                        reAssignTask(worker->id);
                    }
                }else if(worker->last_active + std::chrono::seconds(10) < std::chrono::system_clock::now()){
                    mapreduce::Ping ping = MessageGenerator::Ping();
                    worker->sendMessage(ping);
                }
            }
        }
        if(has_available && jobs.size() > 0){
            workerMtx.unlock();
            std::lock_guard<std::mutex> lock(jobsMtx);
            assignJob(jobs.back());
            jobs.pop_back();
        }
    }
}