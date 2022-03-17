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
};

#endif