#include "workermanager.h"
#include "protoutils.hpp"
#include <spdlog/spdlog.h>
#include <asio.hpp>

WorkerManager::WorkerManager()
{
}


WorkerManager::~WorkerManager()
{
}


void WorkerManager::operator()(){
    asio::io_context ctx;
    asio::ip::tcp::endpoint ep{asio::ip::tcp::v4(), port};
    asio::ip::tcp::acceptor acceptor(ctx, ep);
    asio::ip::tcp::socket socket(ctx);
    while(true){
        spdlog::info("Waiting for worker");
        acceptor.listen();
        acceptor.accept(socket);
        Pipe pipe(std::move(socket));
        int worker_id = generateWorkerId();
        mapreduce::WorkerAssignment msg = generateWorkerAssignment(worker_id);
        pipe << msg;
        std::string line;
        pipe >> line;
        if(std::stoi(line) == worker_id){
            workers[worker_id] = WorkerObject(&pipe, worker_id);
            spdlog::info("Worker {} connected", worker_id);
        }
    }
}


int WorkerManager::generateWorkerId(){
    return workers.size();
}