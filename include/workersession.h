#ifndef WORKERSESSION_H
#define WORKERSESSION_H

#include "workermanager.h"
#include "pipe.hpp"
#include "protoutils.hpp"

class WorkerSession : public WorkerObject,
                      public std::enable_shared_from_this<WorkerObject>{ 
    WorkerManager &manager;
    std::thread* reciveThread;
    Pipe pipe;

    void readMessage();

    bool assignID();

    public:
        WorkerSession(WorkerManager &manager, 
                      asio::ip::tcp::socket socket,
                      int id);

        ~WorkerSession();

        void start();
};

#endif