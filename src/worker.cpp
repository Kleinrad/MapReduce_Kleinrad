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


void Worker::waitForTask(){
    mapreduce::MessageType type = pipe.reciveMessageType();
    if(type == mapreduce::MessageType::TASK_MAP){
        mapreduce::TaskMap task;
        pipe >> task;
        spdlog::info("Worker {} received task {}", worker_id, task.data());
        waitForTask();
    }else if(type == mapreduce::MessageType::SIGN_OFF){
        signOff();
    }else{
        spdlog::error("Worker {} received invalid message type ({})", worker_id, type);
    }
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
    mapreduce::SignOff signOff = MessageGenerator::SignOff(worker_id, mapreduce::ConnectionType::WORKER);
    pipe.sendMessage(signOff);
    spdlog::info("Worker {} sign off", worker_id);
    exit(0);
}

int main(){
    asio::io_service ctx;
    asio::ip::tcp::endpoint ep{
        asio::ip::address::from_string("127.0.0.1"), 1500};
    asio::ip::tcp::socket socket(ctx);
    socket.connect(ep);
    Worker worker(std::move(socket));
    worker.signOn();
}