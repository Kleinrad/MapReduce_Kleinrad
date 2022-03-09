#include <spdlog/spdlog.h>
#include <thread>
#include "pipe.hpp"
#include "protoutils.hpp"
#include "workersession.h"
#include "master.h"


Master::Master(WorkerManager &manager, 
                asio::ip::tcp::endpoint ep,
                asio::io_context &ctx) 
    : acceptor(ctx, ep), manager(manager){}


Master::~Master(){}


void Master::acceptConnection(){
    spdlog::info("Waiting for Connection");
    acceptor.async_accept(
        [this](const asio::error_code &ec, asio::ip::tcp::socket socket){
        if(ec){
            spdlog::error("Error accepting worker {}", ec.message());
            return;
        }
        int id = manager.generateWorkerId();
        std::make_shared<WorkerSession>(manager, std::move(socket),
            id)->start();
        acceptConnection();
    });
}

int main(){
    asio::io_context ctx;
    asio::ip::tcp::endpoint ep{asio::ip::address_v4(), 1500};
    spdlog::set_level(spdlog::level::debug);
    WorkerManager manager;
    Master master(manager, ep, ctx);
    master.acceptConnection();
    ctx.run();
}