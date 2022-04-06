/*
author: Kleinrad Fabian
matnr: i17053
file: connectionsession.h
class: 5BHIF
catnr: 07
*/

#ifndef WORKERSESSION_H
#define WORKERSESSION_H

#include "workermanager.h"
#include "clientmanager.h"
#include "pipe.hpp"
#include "protoutils.hpp"
#include "messageQueue.hpp"
#include <thread>

class ConnectionSession : public ConnectionObject,
                      public std::enable_shared_from_this<ConnectionObject>{ 
    WorkerManager &workerManager;
    ClientManager &clientManager;
    std::thread* reciveThread;
    std::thread* queueThread;
    Pipe pipe;
    static std::mutex mtx;
    mapreduce::ConnectionType type;
    MessageQueue msgQueue;

    void readMessage();
    void sendMessage(google::protobuf::Message& messaged);
    void checkMessageQueue();
    bool isConnected();
    void closeConnection();
    bool assignID();
    void auth();

    public:
        ConnectionSession(WorkerManager &WorkerManager, ClientManager &clientManager
                        , asio::ip::tcp::socket socket);

        ~ConnectionSession();

        void start();
};

#endif