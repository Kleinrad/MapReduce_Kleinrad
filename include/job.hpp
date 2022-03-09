#ifndef JOBTYPES_H
#define JOBTYPES_H

#include <string>

enum JobType {
    job_word_count,
    job_letter_count
};

enum JobStatus {
    job_new,
    job_queued,
    job_mapping,
    job_reducing,
    job_done
};

struct Job {
    JobType type;
    std::string data;
    JobStatus status{JobStatus::job_new};
    
    Job(JobType type, std::string data) : type(type), data(data) {}
};

#endif