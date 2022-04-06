/*
author: Kleinrad Fabian
matnr: i17053
file: clientmanager.h
class: 5BHIF
catnr: 07
*/

#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <set>
#include <map>
#include <thread>
#include "pipe.hpp"
#include "job.hpp"
#include "connectionobject.hpp"

class ClientManager{
    std::set<connectionPtr> clients;
    asio::ip::port_type port{1500};
    std::map<int, int> jobClientMap;
    std::mutex mtx;
    int totalConnections{0};

    public:
        ClientManager();
        ~ClientManager();

        void registerJob(int jobId, int clientId);
        void join(connectionPtr client);
        void leave(connectionPtr client);
        void sendResult(int jobId
            , std::map<std::string, int> &result);
        int generateID();
};

#endif