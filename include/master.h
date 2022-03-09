#ifndef MASTER_H
#define MASTER_H

#include <asio.hpp>
#include "workermanager.h"

class Master {
    asio::ip::tcp::acceptor acceptor;
    WorkerManager &manager;

    public:
      Master(WorkerManager &manager,
            asio::ip::tcp::endpoint ep,
            asio::io_context &ctx);
      ~Master();

      void acceptConnection();    
};

#endif