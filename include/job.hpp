#ifndef JOBTYPES_H
#define JOBTYPES_H

#include <string>

enum JobType {
    job_word_count,
    job_letter_count
};

struct Job {
    JobType type;
    std::string data;
    
    Job(JobType type, std::string data) : type(type), data(data) {}
};

#endif