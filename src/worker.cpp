/*
author: Kleinrad Fabian
matnr: i17053
file: worker.cpp
class: 5BHIF
catnr: 07
*/

#include <thread>
#include <fstream>
#include <chrono>
#include <spdlog/spdlog.h>
#include <regex>
#include <json.hpp>
#include "protoutils.hpp"
#include "worker.h"

Worker::Worker(asio::ip::tcp::socket socket):
    pipe(Pipe(std::move(socket))) {}


Worker::~Worker() {
    signOff();
}


void Worker::handleMap(int type, std::string data, int jobId) {
    spdlog::info("Worker {}: handleMap type {}", workerId, type);
    spdlog::debug("Data size {}", data.size());
    std::vector<std::pair<std::string, int>> result;
    if(type == 0){
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
    }else if(type == 1){
        std::regex rgx("\\s+");
        std::sregex_token_iterator iter(data.begin(),
            data.end(),
            rgx,
            -1);
        std::sregex_token_iterator end;
        while(iter != end){
            result.push_back(std::make_pair(*iter, 1));
            iter++;
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

    }
    mapreduce::ResultMap resultMsg = 
        MessageGenerator::ResultMap(result, jobId);
    pipe.sendMessage(resultMsg);
    is_busy = false;
}


void Worker::handleReduce(int type
        , mapreduce::TaskReduce::ReduceData data, int jobId) {
    spdlog::info("Worker {}: handleReduce type {} {}", workerId, type, jobId);
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
        MessageGenerator::ResultReduce(result, jobId);
    pipe.sendMessage(resultMsg);
}


void Worker::waitForTask(){
    mapreduce::MessageType type = pipe.reciveMessageType();
    if(type == mapreduce::MessageType::TASK_MAP && !is_busy){
        mapreduce::TaskMap task;
        pipe >> task;
        std::thread t([this, task](){
            handleMap(task.job_type(), task.data(), task.jobid());
        });
        is_busy = true;
        t.detach();
    }else if(type == mapreduce::MessageType::TASK_REDUCE && !is_busy){
        mapreduce::TaskReduce task;
        pipe >> task;
        std::thread t([this, task](){
            handleReduce(task.job_type(), task.data(), task.jobid());
        });
        is_busy = true;
        t.detach();
    }else if(type == mapreduce::MessageType::SIGN_OFF){
        signOff();
        return;
    }else if(type == mapreduce::MessageType::PING){
        mapreduce::Ping ping;
        pipe >> ping;
        //spdlog::info("Worker {} received ping", workerId);
        pipe.sendMessage(ping);
    }else{
        spdlog::error("Worker {} received invalid message type ({})", workerId, type);
        if(!pipe){
            spdlog::error("Worker {} connection closed", workerId);
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
        workerId = assignment.id();
        mapreduce::Confirm confirm = MessageGenerator::Confirm(workerId
            , mapreduce::ConnectionType::WORKER);
        pipe.sendMessage(confirm);
        spdlog::info("Worker {} sign on", workerId);
        waitForTask();
    }else{
        spdlog::error("Invalid message type");
        throw std::runtime_error("Worker::signOn: Invalid Message recived");
    }
}


void Worker::signOff() {
    if(pipe){
        mapreduce::SignOff signOff = MessageGenerator::SignOff(workerId, mapreduce::ConnectionType::WORKER);
        pipe.sendMessage(signOff);
    }
    spdlog::info("Worker {} sign off", workerId);
    exit(0);
}

int main(){
    std::ifstream f("config.json");
    nlohmann::json j = nlohmann::json::parse(f);

    spdlog::set_level(spdlog::level::info);
    asio::io_service ctx;
    asio::ip::tcp::endpoint ep{
        asio::ip::address::from_string(j["master"]), (asio::ip::port_type)j["port"]};
    asio::ip::tcp::socket socket(ctx);
    socket.connect(ep);
    Worker worker(std::move(socket));
    worker.signOn();
}