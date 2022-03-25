#include <thread>
#include <chrono>
#include <spdlog/spdlog.h>
#include "protoutils.hpp"
#include "worker.h"

Worker::Worker(asio::ip::tcp::socket socket):
    pipe(Pipe(std::move(socket))) {}


Worker::~Worker() {
    signOff();
}


void Worker::handleMap(int type, std::string data, int job_id) {
    spdlog::info("Worker {}: handleMap type {}", worker_id, type);
    spdlog::debug("Data size {}", data.size());
    std::vector<std::pair<std::string, int>> result;
    for(int i = 0; i < (int)data.size(); i++){
        result.push_back(std::make_pair(data.substr(i, 1), 1));
    }
    spdlog::debug("map size {}", result.size());
    std::map<std::string, int> result_map;
    for(auto &r: result){
        result_map[r.first] += r.second;
    }
    int sum = 0;
    result.clear();
    for(auto &r: result_map){
        sum += r.second;
        result.push_back(std::make_pair(r.first, r.second));
    }
    mapreduce::ResultMap resultMsg = 
        MessageGenerator::ResultMap(result, job_id);
    pipe.sendMessage(resultMsg);
    is_busy = false;
}


void Worker::handleReduce(int type
        , mapreduce::TaskReduce::ReduceData data, int job_id) {
    spdlog::info("Worker {}: handleReduce type {} {}", worker_id, type, job_id);
    spdlog::debug("Data size reduce {}", data.values().size());
    std::map<std::string, int> result;
    for(auto &pair : data.values()){
        if(result.find(pair.key()) == result.end()){
            result[pair.key()] = pair.value();
        }else{
            result[pair.key()] += pair.value();
        }
    }
    is_busy = false;
    mapreduce::ResultReduce resultMsg = 
        MessageGenerator::ResultReduce(result, job_id);
    pipe.sendMessage(resultMsg);
}


void Worker::waitForTask(){
    mapreduce::MessageType type = pipe.reciveMessageType();
    if(type == mapreduce::MessageType::TASK_MAP && !is_busy){
        mapreduce::TaskMap task;
        pipe >> task;
        std::thread t([this, task](){
            handleMap(task.job_type(), task.data(), task.job_id());
        });
        is_busy = true;
        t.detach();
    }else if(type == mapreduce::MessageType::TASK_REDUCE && !is_busy){
        mapreduce::TaskReduce task;
        pipe >> task;
        std::thread t([this, task](){
            handleReduce(task.job_type(), task.data(), task.job_id());
        });
        is_busy = true;
        t.detach();
    }else if(type == mapreduce::MessageType::SIGN_OFF){
        signOff();
        return;
    }else if(type == mapreduce::MessageType::PING){
        mapreduce::Ping ping;
        pipe >> ping;
        //spdlog::info("Worker {} received ping", worker_id);
        pipe.sendMessage(ping);
    }else{
        spdlog::error("Worker {} received invalid message type ({})", worker_id, type);
        if(!pipe){
            spdlog::error("Worker {} connection closed", worker_id);
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    waitForTask();
}


void Worker::signOn(){
    mapreduce::Authentication auth = MessageGenerator::Authentication(
        mapreduce::ConnectionType::WORKER);
    pipe.sendMessage(auth);
    if(pipe.reciveMessageType() == mapreduce::MessageType::ASSIGNMENT){
        mapreduce::Assignment assignment;
        pipe >> assignment;
        worker_id = assignment.id();
        mapreduce::Confirm confirm = MessageGenerator::Confirm(worker_id
            , mapreduce::ConnectionType::WORKER);
        pipe.sendMessage(confirm);
        spdlog::info("Worker {} sign on", worker_id);
        waitForTask();
    }else{
        spdlog::error("Invalid message type");
        throw std::runtime_error("Worker::signOn: Invalid Message recived");
    }
}


void Worker::signOff() {
    if(pipe){
        mapreduce::SignOff signOff = MessageGenerator::SignOff(worker_id, mapreduce::ConnectionType::WORKER);
        pipe.sendMessage(signOff);
    }
    spdlog::info("Worker {} sign off", worker_id);
    exit(0);
}

int main(){
    spdlog::set_level(spdlog::level::debug);
    asio::io_service ctx;
    asio::ip::tcp::endpoint ep{
        asio::ip::address::from_string("127.0.0.1"), 1500};
    asio::ip::tcp::socket socket(ctx);
    socket.connect(ep);
    Worker worker(std::move(socket));
    worker.signOn();
}