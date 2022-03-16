#include "connectionsession.h"
#include <chrono>


ConnectionSession::ConnectionSession(WorkerManager &workerManager, 
                                    ClientManager &clientManager,
                            asio::ip::tcp::socket socket) 
    : ConnectionObject(), workerManager(workerManager), 
    clientManager(clientManager), pipe(Pipe(std::move(socket))) {
        last_active = std::chrono::system_clock::now();
}


ConnectionSession::~ConnectionSession(){
    spdlog::debug("ConnectionSession Deconst");
    pipe.~Pipe();
    spdlog::debug("ConnectionSession Deconst ended");
}


void ConnectionSession::sendMessage(
            google::protobuf::Message& message){
    pipe.sendMessage(message);
}


void ConnectionSession::readMessage(){
    if(pipe){
        mapreduce::MessageType type = pipe.reciveMessageType();
        last_active = std::chrono::system_clock::now();
        if(type == mapreduce::MessageType::SIGN_OFF){
            if(this->type == mapreduce::ConnectionType::WORKER){
                workerManager.leave(shared_from_this());
                return;
            }else if(this->type == mapreduce::ConnectionType::CLIENT){
                clientManager.leave(shared_from_this());
                return;
            }
        }if(type == mapreduce::MessageType::JOB_REQUEST){
            mapreduce::JobRequest jobRequest;
            pipe >> jobRequest;
            Job job(jobRequest.job_type(), jobRequest.data());
            clientManager.registerJob(job.id, id);
            workerManager.assignJob(job);
        }
        if(type == mapreduce::MessageType::PING){
            mapreduce::Ping p;
            pipe >> p;
            spdlog::debug("{} received ping", id);
        }
        readMessage();
    }else{
        if(this->type == mapreduce::ConnectionType::WORKER){
            workerManager.leave(shared_from_this());
            return;
        }else if(this->type == mapreduce::ConnectionType::CLIENT){
            clientManager.leave(shared_from_this());
            return;
        }
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
        type = auth.connection_type();
    }else{
        spdlog::error("Worker {} received invalid connection type ", id);
    }
}


bool ConnectionSession::isConnected(){
    return pipe.operator bool();
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