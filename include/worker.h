#ifndef WORKER_H
#define WORKER_H

#include <asio.hpp>
#include "pipe.hpp"

class Worker {
    int worker_id;
    Pipe* pipe;

    void waitForTask();
    public:
        Worker(Pipe* pipe);
        ~Worker();

        void signOff();
        void signOn();
};

#endif