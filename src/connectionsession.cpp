#include "connectionsession.h"


ConnectionSession::ConnectionSession(WorkerManager &workerManager, 
                                    ClientManager &clientManager,
                            asio::ip::tcp::socket socket) 
    : ConnectionObject(), workerManager(workerManager), 
    clientManager(clientManager), pipe(Pipe(std::move(socket))) {}


ConnectionSession::~ConnectionSession(){}


void ConnectionSession::readMessage(){
    mapreduce::MessageType type = pipe.reciveMessageType();
    if(type == mapreduce::MessageType::SIGN_OFF){
        if(this->type == mapreduce::ConnectionType::WORKER){
            spdlog::info("Worker {} sign off", id);
            workerManager.leave(shared_from_this());
            return;
        }else if(this->type == mapreduce::ConnectionType::CLIENT){
            spdlog::info("Client {} sign off", id);
            clientManager.leave(shared_from_this());
            return;
        }
    }if(type == mapreduce::MessageType::JOB_REQUEST){
        mapreduce::JobRequest jobRequest;
        pipe >> jobRequest;
        Job job(jobRequest.job_type(), jobRequest.data(), jobRequest.id());
        workerManager.assignJob(job);
    }
}


bool ConnectionSession::assignID(){
    mapreduce::Assignment assignment = MessageGenerator::Assignment(id, type);
    pipe.sendMessage(assignment);
    mapreduce::MessageType type = pipe.reciveMessageType();
    if(type == mapreduce::MessageType::CONFIRM){
        mapreduce::Confirm confirm;
        pipe >> confirm;
        if(confirm.id() == id){
            return true;
        }else{
            spdlog::error("Confirmation failed: Invalid id");
        }
    }else{
        spdlog::error("Confirmation failed: Invalid message type ({})", type);
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