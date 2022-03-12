#ifndef MASTER_H
#define MASTER_H

#include <asio.hpp>
#include "workermanager.h"
#include "clientmanager.h"

class Master {
    asio::ip::tcp::acceptor acceptor;
    WorkerManager &workerManager;
    ClientManager &clientManager;

    public:
      Master(WorkerManager &workerManager,
            ClientManager &clientManager,
            asio::ip::tcp::endpoint ep,
            asio::io_context &ctx);
      ~Master();

      void acceptConnection();    
};

#endif