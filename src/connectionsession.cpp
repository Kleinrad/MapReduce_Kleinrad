#include "connectionsession.h"


ConnectionSession::ConnectionSession(WorkerManager &workerManager, 
                                    ClientManager &clientManager,
                            asio::ip::tcp::socket socket) 
    : ConnectionObject(), workerManager(workerManager), 
    clientManager(clientManager), pipe(Pipe(std::move(socket))) {}


ConnectionSession::~ConnectionSession(){}


void ConnectionSession::readMessage(){
    mapreduce::MessageType type = pipe.reciveMessageType();
    if(type == mapreduce::MessageType::WORKER_SIGN_OFF){
        if(this->type == mapreduce::ConnectionType::WORKER){
            spdlog::info("Worker {} sign off", id);
            workerManager.leave(shared_from_this());
            return;
        }else if(this->type == mapreduce::ConnectionType::CLIENT){
            spdlog::info("Client {} sign off", id);
            clientManager.leave(shared_from_this());
            return;
        }
        spdlog::info("Worker {} sign off", id);
    }
}


bool ConnectionSession::assignID(){
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


void ConnectionSession::auth(){
    if(pipe.reciveMessageType() == mapreduce::MessageType::AUTHENTICATION){
        mapreduce::Authentication auth;
        pipe >> auth;
        type = auth.type();
    }else{
        spdlog::error("Worker {} received invalid connection type ", id);
    }
}


void ConnectionSession::start(){
    auth();
    if(type == mapreduce::ConnectionType::WORKER){
        id = workerManager.generateID();
    }else if(type == mapreduce::ConnectionType::CLIENT){
        id = clientManager.generateID();
    }
    if(assignID()){
        if(type == mapreduce::ConnectionType::WORKER){
            workerManager.join(shared_from_this());
        }else if(type == mapreduce::ConnectionType::CLIENT){
            clientManager.join(shared_from_this());
        }
        reciveThread = new std::thread([this](){
            readMessage();
        });
        reciveThread->detach();
    }
}