#ifndef WORKER_H
#define WORKER_H

#include <asio.hpp>

class Worker {
    asio::ip::tcp::socket* socket;

    public:
        Worker(asio::ip::tcp::socket* socket);
        ~Worker();
};

#endif