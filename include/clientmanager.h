#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <set>
#include <thread>
#include "pipe.hpp"
#include "job.hpp"
#include "connectionobject.hpp"

class ClientManager{
    std::set<connection_ptr> clients;
    asio::ip::port_type port{1500};
    std::mutex mtx;
    int totalConnections{0};

    public:
        ClientManager();
        ~ClientManager();

        void join(connection_ptr worker);
        void leave(connection_ptr worker);
        int generateID();
};

#endif