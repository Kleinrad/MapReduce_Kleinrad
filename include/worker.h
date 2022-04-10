/*
author: Kleinrad Fabian
matnr: i17053
file: worker.h
class: 5BHIF
catnr: 07
*/

#ifndef WORKER_H
#define WORKER_H

#include <asio.hpp>
#include <list>
#include "pipe.hpp"

class Worker {
    int workerId;
    bool is_busy{false};
    Pipe pipe;

    void waitForTask();
    void handleMap(int type, std::string data, int jobId);
    void handleReduce(int type
        , mapreduce::TaskReduce::ReduceData data, int jobId);

  public:
    Worker(asio::ip::tcp::socket socket);
    ~Worker();

    void signOff();
    void signOn();
};

#endif