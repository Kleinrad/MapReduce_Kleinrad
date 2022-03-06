#include "workermanager.h"
#include "protoutils.hpp"
#include <spdlog/spdlog.h>
#include <asio.hpp>

WorkerManager::WorkerManager(asio::io_context &ctx, asio::ip::tcp::endpoint ep)
: acceptor(ctx, ep) {
    acceptor.listen();
}


WorkerManager::~WorkerManager()
{
}


void WorkerManager::acceptWorker(){
    spdlog::info("Waiting for worker");
    acceptor.async_accept(
        [this](const asio::error_code &ec, asio::ip::tcp::socket socket){
        if(ec){
            spdlog::error("Error accepting worker {}", ec.message());
            return;
        }
        int id = generateWorkerId();
        Pipe pipe(std::move(socket));
        mapreduce::WorkerAssignment assignment = generateWorkerAssignment(id);
        pipe.sendMessage(assignment);
        mapreduce::MessageType type = pipe.reciveMessageType();
        if(type == mapreduce::MessageType::CONFIRM){
            mapreduce::Confirm confirm;
            pipe >> confirm;
            if(confirm.worker_id() == id){
                std::make_shared<WorkerSession>(*this, std::move(socket),
                    id)->start();
                spdlog::info("Worker {} connected", id);
            }else{
                spdlog::error("Worker confirmation vailed: Invalid worker id");
            }
        }else{
            spdlog::error("Worker confirmation vailed: Invalid message type ({})", type);
        }
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