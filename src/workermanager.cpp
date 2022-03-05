#include "workermanager.h"
#include "protoutils.hpp"
#include <spdlog/spdlog.h>
#include <asio.hpp>

WorkerManager::WorkerManager(asio::io_context &ctx, asio::ip::tcp::endpoint ep)
: acceptor(ctx, ep) {}


WorkerManager::~WorkerManager()
{
}


void WorkerManager::acceptWorker(){
    spdlog::info("Waiting for worker");
    acceptor.listen();
    acceptor.async_accept(
        [this](const asio::error_code &ec, asio::ip::tcp::socket socket){
        if(ec){
            spdlog::error("Error accepting worker {}", ec.message());
            return;
        }
        std::make_shared<WorkerSession>(*this, std::move(socket),
            generateWorkerId())->start();
        spdlog::info("Worker connected");
        acceptWorker();
    });
}


void WorkerManager::join(worker_ptr worker)
{
    workers.insert(worker);
}


int WorkerManager::generateWorkerId(){
    return workers.size();
}