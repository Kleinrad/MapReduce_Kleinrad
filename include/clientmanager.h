#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <set>
#include <map>
#include <thread>
#include "pipe.hpp"
#include "job.hpp"
#include "connectionobject.hpp"

class ClientManager{
    std::set<connection_ptr> clients;
    asio::ip::port_type port{1500};
    std::map<int, int> job_client_map;
    std::mutex mtx;
    int totalConnections{0};

    public:
        ClientManager();
        ~ClientManager();

        void registerJob(int job_id, int client_id);
        void join(connection_ptr worker);
        void leave(connection_ptr worker);
        void sendResult(int job_id
            , std::map<std::string, int> &result);
        int generateID();
};

#endif