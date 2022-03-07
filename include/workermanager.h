#ifndef WORKMANAGER_H
#define WORKMANAGER_H

#include <set>
#include "job.hpp"
#include "pipe.hpp"

class WorkerObject {
    protected:
        int id{-1};
        bool is_available;

    public:
        WorkerObject(int id) 
        : id(id), is_available(true) {};

};

typedef std::shared_ptr<WorkerObject> worker_ptr;

class WorkerManager{
    std::set<worker_ptr> workers;
    asio::ip::port_type port{1500};
    asio::ip::tcp::acceptor acceptor;

    int generateWorkerId();

    public:
        WorkerManager(asio::io_context &ctx
                    , asio::ip::tcp::endpoint ep);
        ~WorkerManager();

        void acceptWorker();
        void join(worker_ptr worker);
        bool assignJob(Job job);
};

class WorkerSession : public WorkerObject,
                      public std::enable_shared_from_this<WorkerObject>{ 
    WorkerManager &manager;
    std::thread* reciveThread;
    Pipe pipe;

    void readMessage(){
        mapreduce::MessageType type = pipe.reciveMessageType();
        if(type == mapreduce::MessageType::WORKER_SIGN_OFF){
            spdlog::info("Worker {} sign off", id);
        }
    }

    public:
        WorkerSession(WorkerManager &manager, 
                      asio::ip::tcp::socket socket,
                      int id) 
                : WorkerObject(id), manager(manager),
                  pipe(Pipe(std::move(socket))) {};

        ~WorkerSession(){
        }

        void start(){
            manager.join(shared_from_this());
            reciveThread = new std::thread([this](){
                readMessage();
            });
            reciveThread->detach();
        }
};

#endif