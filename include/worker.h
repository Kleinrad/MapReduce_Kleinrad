#ifndef WORKER_H
#define WORKER_H

#include <asio.hpp>
#include "pipe.hpp"

class Worker {
    int worker_id;
    bool is_busy{false};
    Pipe pipe;

    void waitForTask();
    void handleMap(int type, std::string data, int job_id);
    void handleReduce(int type
        , std::set<std::pair<std::string, int>> data, int job_id);
    public:
        Worker(asio::ip::tcp::socket socket);
        ~Worker();

        void signOff();
        void signOn();
};

#endif