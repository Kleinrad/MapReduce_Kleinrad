#ifndef JOBTYPES_H
#define JOBTYPES_H

#include <string>
#include "Message.pb.h"

enum JobStatus {
    job_new,
    job_queuedMap,
    job_mapping,
    job_mapped,
    job_queuedReduce,
    job_reducing,
    job_done
};


struct ActiveJobStruct {
    std::map<int, std::string> workerData;
    std::vector<std::pair<std::string, int>> results;
    int job_id;
    bool is_active{false};
    JobStatus status;
    mapreduce::JobType type;

    ActiveJobStruct() {}

    ActiveJobStruct(int id, JobStatus status, mapreduce::JobType type) 
        : job_id(id), status(status), type(type) {}

    void addWorker(int worker, std::string data) {
        workerData.insert({worker, data});
        is_active = true;
    }

    void removeWorker(int worker) {
        workerData.erase(worker);
        if(workerData.size() == 0) {
            is_active = false;
        }
    }

    void addResults(std::vector<std::pair<std::string, int>> results) {
        this->results.insert(this->results.end(), results.begin(), results.end());
    }

    bool contains(int worker) {
        return workerData.find(worker) != workerData.end();
    }

    std::string getWorkerData(int worker) {
        if(workerData.find(worker) == workerData.end()) {
            spdlog::error("Worker {} not found", worker);
            return "";
        }
        return workerData[worker];
    }
};

class Job {
    static int job_counter;

public:
    int id;
    mapreduce::JobType type;
    std::string data;
    JobStatus status{JobStatus::job_new};
    std::vector<std::pair<std::string, int>> results;
    int mappers{-1};
    int reducers{-1};
    
    Job(mapreduce::JobType type, std::string data) 
        : id(job_counter++), type(type), data(data) {}

    Job(mapreduce::JobType type, std::string data, int id)
        : id(id), type(type), data(data) {}

    Job(ActiveJobStruct activeJob, int worker_id)
        : id(activeJob.job_id), type(activeJob.type), 
          status(activeJob.status) {
              data = activeJob.getWorkerData(worker_id);
    }

    Job(ActiveJobStruct activeJob)
        : id(activeJob.job_id), type(activeJob.type), 
          status(activeJob.status), results(activeJob.results) {}
};

struct ActiveJob : public ActiveJobStruct {
    ActiveJob() {}

    ActiveJob(Job job)
        : ActiveJobStruct(job.id, job.status, job.type) {}

    bool isActive(){
        return is_active;
    }
};

#endif