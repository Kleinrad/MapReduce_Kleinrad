/*
author: Kleinrad Fabian
matnr: i17053
file: master.cpp
class: 5BHIF
catnr: 07
*/

#include <spdlog/spdlog.h>
#include <fstream>
#include <thread>
#include <json.hpp>
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
    std::ifstream f("config.json");
    nlohmann::json j = nlohmann::json::parse(f);

    asio::io_context ctx;
    asio::ip::tcp::endpoint ep{asio::ip::address_v4()
        , (asio::ip::port_type)j["port"]};
    spdlog::set_level(spdlog::level::info);
    WorkerManager workerManager;
    ClientManager clientManager;
    Master master(workerManager, clientManager, ep, ctx);
    spdlog::info("Waiting for Connection");
    master.acceptConnection();
    ctx.run();
}