#include "workersession.h"


WorkerSession::WorkerSession(WorkerManager &manager, 
                            asio::ip::tcp::socket socket,
                            int id) 
    : WorkerObject(id), manager(manager), 
    pipe(Pipe(std::move(socket))) {}


WorkerSession::~WorkerSession(){}


void WorkerSession::readMessage(){
    mapreduce::MessageType type = pipe.reciveMessageType();
    if(type == mapreduce::MessageType::WORKER_SIGN_OFF){
        manager.leave(shared_from_this());
        spdlog::info("Worker {} sign off", id);
    }
}


bool WorkerSession::assignID(){
    mapreduce::WorkerAssignment assignment = MessageGenerator::WorkerAssignment(id);
    pipe.sendMessage(assignment);
    mapreduce::MessageType type = pipe.reciveMessageType();
    if(type == mapreduce::MessageType::CONFIRM){
        mapreduce::Confirm confirm;
        pipe >> confirm;
        if(confirm.worker_id() == id){
            spdlog::info("Worker {} connected", id);
            return true;
        }else{
            spdlog::error("Worker confirmation vailed: Invalid worker id");
        }
    }else{
        spdlog::error("Worker confirmation vailed: Invalid message type ({})", type);
    }
    return false;
}


void WorkerSession::start(){
    if(assignID()){
        manager.join(shared_from_this());
        reciveThread = new std::thread([this](){
            readMessage();
        });
        reciveThread->detach();
    }
}