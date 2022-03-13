#ifndef JOBTYPES_H
#define JOBTYPES_H

#include <string>
#include "Message.pb.h"

enum JobStatus {
    job_new,
    job_queued,
    job_mapping,
    job_reducing,
    job_done
};

struct Job {
    mapreduce::JobType type;
    std::string data;
    JobStatus status{JobStatus::job_new};
    
    Job(mapreduce::JobType type, std::string data) 
        : type(type), data(data) {}
};

#endif