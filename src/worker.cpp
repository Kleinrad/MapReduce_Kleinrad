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
    }else if(type == mapreduce::MessageType::WORKER_SIGN_OFF){
        signOff();
    }else{
        spdlog::error("Worker {} received invalid message type ({})", worker_id, type);
    }
}


void Worker::signOn(){
    if(pipe.reciveMessageType() == mapreduce::MessageType::WORKER_ASSIGNMENT){
        mapreduce::WorkerAssignment assignment;
        pipe >> assignment;
        worker_id = assignment.worker_id();
        mapreduce::Confirm confirm = generateConfirm(worker_id);
        pipe.sendMessage(confirm);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        signOff();
        //waitForTask();
    }else{
        spdlog::error("Invalid message type");
        throw std::runtime_error("Worker::signOn: Invalid Message recived");
    }
}


void Worker::signOff() {
    mapreduce::WorkerSignOff signOff = generateWorkerSignOff(worker_id);
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