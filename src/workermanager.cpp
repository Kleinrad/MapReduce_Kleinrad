#include "workermanager.h"
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
        acceptor.listen();
        acceptor.accept(socket);
        Pipe pipe(std::move(socket));
        std::string worker_id = "" + generateWorkerId();
        pipe << worker_id;
        std::string line;
        pipe >> line;
        if(line == worker_id){
            int wId = std::stoi(worker_id);
            workers[wId] = WorkerObject(pipe, wId);
            spdlog::info("Worker {} connected", worker_id);
        }
    }
}


int WorkerManager::generateWorkerId(){
    return workers.size();
}