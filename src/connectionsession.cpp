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
}


void ConnectionSession::sendMessage(
            google::protobuf::Message& message){
    std::lock_guard<std::mutex> lock(mtx);
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
        }
        if(type == mapreduce::MessageType::RESULT_MAP){
            mapreduce::ResultMap resultMap;
            pipe >> resultMap;
            std::vector<std::pair<std::string, int>> result;
            for(auto& r : resultMap.values()){
                result.push_back(std::make_pair(r.key(), r.value()));
            }
            is_available = true;
            workerManager.mapResult(resultMap.job_id(), id, result);
        }
        readMessage();
    }else{
        if(this->type == mapreduce::ConnectionType::WORKER){
            if(!is_available){
                workerManager.reAssignTask(id);
            }
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
    std::lock_guard<std::mutex> lock(mtx);
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
    std::lock_guard<std::mutex> lock(mtx);
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


void ConnectionSession::closeConnection(){
    pipe.close();
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