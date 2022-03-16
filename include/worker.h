#ifndef WORKER_H
#define WORKER_H

#include <asio.hpp>
#include "pipe.hpp"

class Worker {
    int worker_id;
    bool is_busy{false};
    Pipe pipe;

    void waitForTask();
    void handleMap(int type, std::string data);
    public:
        Worker(asio::ip::tcp::socket socket);
        ~Worker();

        void signOff();
        void signOn();
};

#endif