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


struct ActiveJob {
    std::map<int, std::string> workerData;
    int job_id;
    JobStatus status;
    mapreduce::JobType type;

    ActiveJob(int id, JobStatus status, mapreduce::JobType type) 
        : status(status), type(type), job_id(id) {}

    void addWorker(int worker, std::string data) {
        workerData.insert({worker, data});
    }

    bool contains(int worker) {
        return workerData.find(worker) != workerData.end();
    }

    std::string getWorkerData(int worker) {
        if(workerData.find(worker) == workerData.end()) {
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
    int mappers{-1};
    int reducers{-1};
    
    Job(mapreduce::JobType type, std::string data) 
        : id(job_counter++), type(type), data(data) {}

    Job(mapreduce::JobType type, std::string data, int id)
        : id(id), type(type), data(data) {}

    Job(ActiveJob activeJob, int worker_id)
        : id(activeJob.job_id), type(activeJob.type), 
          status(activeJob.status) {
              data = activeJob.getWorkerData(worker_id);
    }
};

#endif