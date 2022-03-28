#include "connectionsession.h"
#include <chrono>


std::mutex ConnectionSession::mtx;


ConnectionSession::ConnectionSession(WorkerManager &workerManager, 
                                    ClientManager &clientManager,
                            asio::ip::tcp::socket socket) 
    : ConnectionObject(), workerManager(workerManager), 
    clientManager(clientManager), pipe(Pipe(std::move(socket))) {
        last_active = std::chrono::system_clock::now();
        queueThread = new std::thread(&ConnectionSession::checkMessageQueue, this);
        queueThread->detach();
}


ConnectionSession::~ConnectionSession(){
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
            QueueItem item{mapreduce::MessageType::JOB_REQUEST};
            item.jobType = jobRequest.job_type();
            item.dataRaw = new char[jobRequest.data().size()];
            std::strcpy(item.dataRaw, jobRequest.data().c_str());
            msgQueue.push(&item);
        }
        if(type == mapreduce::MessageType::PING){
            mapreduce::Ping p;
            pipe >> p;
        }
        if(type == mapreduce::MessageType::RESULT_MAP){
            mapreduce::ResultMap resultMap;
            pipe >> resultMap;
            QueueItem item{mapreduce::MessageType::RESULT_MAP};
            item.dataReduce->reserve(resultMap.values().size());
            for(auto& r : resultMap.values()){
                item.dataReduce->push_back(std::make_pair(r.key(), r.value()));
            }
            item.id = resultMap.job_id();
            msgQueue.push(&item);
        }
        if(type == mapreduce::MessageType::RESULT_REDUCE){
            mapreduce::ResultReduce resultReduce;
            pipe >> resultReduce;
            QueueItem item{mapreduce::MessageType::RESULT_REDUCE};
            for(auto& r : resultReduce.values()){
                item.dataResult->insert(std::make_pair(r.key(), r.value()));
            }
            item.id = resultReduce.job_id();
            msgQueue.push(&item);
        }
        if(type == mapreduce::MessageType::CONFIRM){
            mapreduce::Confirm confirm;
            pipe >> confirm;
            QueueItem item{mapreduce::MessageType::CONFIRM};
            item.id = confirm.id();
            item.connectionType = confirm.connection_type();
            msgQueue.push(&item);
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


void ConnectionSession::checkMessageQueue(){
    while (true)
    {
        QueueItem item = *(msgQueue.pop());
        std::lock_guard<std::mutex> lock(mtx);
        mapreduce::MessageType type = item.type;
        if(type == mapreduce::MessageType::SIGN_OFF){
            if(this->type == mapreduce::ConnectionType::WORKER){
                workerManager.leave(shared_from_this());
                return;
            }else if(this->type == mapreduce::ConnectionType::CLIENT){
                clientManager.leave(shared_from_this());
                return;
            }
        }if(type == mapreduce::MessageType::JOB_REQUEST){
            Job job(item.jobType, item.dataRaw);
            clientManager.registerJob(job.id, id);
            workerManager.assignJob(job);
        }
        if(type == mapreduce::MessageType::RESULT_MAP){
            is_available = true;
            workerManager.mapResult(item.id, id, *(item.dataReduce));
        }
        if(type == mapreduce::MessageType::RESULT_REDUCE){
            is_available = true;
            if(workerManager.reduceResult(item.id, id, *(item.dataResult))){
                clientManager.sendResult(item.id, *(item.dataResult));
            }
        }
        if(type == mapreduce::MessageType::CONFIRM){
            if(item.connectionType == mapreduce::ConnectionType::WORKER){
            }
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