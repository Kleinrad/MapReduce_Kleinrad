#ifndef JOBTYPES_H
#define JOBTYPES_H

#include <string>

enum JobType {
    job_word_count,
    job_letter_count
};

class Job {
    public:
        Job(JobType type, std::string data) : type(type), data(data) {}
        JobType type;
        std::string data;
};

#endif