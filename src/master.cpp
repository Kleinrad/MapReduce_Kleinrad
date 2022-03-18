#include <spdlog/spdlog.h>
#include <thread>
#include "pipe.hpp"
#include "protoutils.hpp"
#include "connectionsession.h"
#include "master.h"


Master::Master(WorkerManager &workerManager,
                ClientManager &clientManager,
                asio::ip::tcp::endpoint ep,
                asio::io_context &ctx) 
    : acceptor(ctx, ep), workerManager(workerManager),
    clientManager(clientManager){}


Master::~Master(){}


void Master::acceptConnection(){
    acceptor.async_accept(
        [this](const asio::error_code &ec, asio::ip::tcp::socket socket){
        if(ec){
            spdlog::error("Error accepting worker {}", ec.message());
            return;
        }
        std::make_shared<ConnectionSession>(workerManager, clientManager,
            std::move(socket))->start();
        acceptConnection();
    });
}

int main(){
    asio::io_context ctx;
    asio::ip::tcp::endpoint ep{asio::ip::address_v4(), 1500};
    spdlog::set_level(spdlog::level::debug);
    WorkerManager workerManager;
    ClientManager clientManager;
    Master master(workerManager, clientManager, ep, ctx);
    spdlog::info("Waiting for Connection");
    master.acceptConnection();
    ctx.run();
}