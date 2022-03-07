#ifndef WORKER_H
#define WORKER_H

#include <asio.hpp>
#include "pipe.hpp"

class Worker {
    int worker_id;
    Pipe pipe;

    void waitForTask();
    public:
        Worker(asio::ip::tcp::socket socket);
        ~Worker();

        void signOff();
        void signOn();
};

#endif