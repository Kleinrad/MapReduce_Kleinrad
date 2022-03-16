#ifndef WORKERSESSION_H
#define WORKERSESSION_H

#include "workermanager.h"
#include "clientmanager.h"
#include "pipe.hpp"
#include "protoutils.hpp"
#include <thread>

class ConnectionSession : public ConnectionObject,
                      public std::enable_shared_from_this<ConnectionObject>{ 
    WorkerManager &workerManager;
    ClientManager &clientManager;
    std::thread* reciveThread;
    Pipe pipe;
    std::mutex mtx;
    mapreduce::ConnectionType type;

    void readMessage();
    void sendMessage(google::protobuf::Message& message);
    bool isConnected();
    bool assignID();
    void auth();

    public:
        ConnectionSession(WorkerManager &WorkerManager, ClientManager &clientManager
                        , asio::ip::tcp::socket socket);

        ~ConnectionSession();

        void start();
};

#endif