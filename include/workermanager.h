#ifndef WORKMANAGER_H
#define WORKMANAGER_H

#include "job.hpp"

class WorkManager{
    public:
        WorkManager();
        ~WorkManager();

        void operator()();
        bool assignJob(Job job);
};

#endif