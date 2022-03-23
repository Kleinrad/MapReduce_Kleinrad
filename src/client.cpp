#include <spdlog/spdlog.h>
#include <thread>
#include "protoutils.hpp"
#include "job.hpp"
#include "client.h"
#include <fstream>

int Job::job_counter = 0;

Client::Client(asio::ip::tcp::socket socket):
    pipe(Pipe(std::move(socket))) {}


Client::~Client() {
    signOff();
}


void Client::signOn(){
    mapreduce::Authentication auth = MessageGenerator::Authentication(
        mapreduce::ConnectionType::CLIENT);
    pipe.sendMessage(auth);
    if(pipe.reciveMessageType() == mapreduce::MessageType::ASSIGNMENT){
        mapreduce::Assignment assignment;
        pipe >> assignment;
        client_id = assignment.id();
        mapreduce::Confirm confirm = MessageGenerator::Confirm(client_id
            , mapreduce::ConnectionType::CLIENT);
        pipe.sendMessage(confirm);
        spdlog::info("Client {} sign on", client_id);
    }else{
        spdlog::error("Invalid message type");
        throw std::runtime_error("Client::signOn: Invalid Message recived");
    }
}


void Client::signOff() {
    if(pipe){
        mapreduce::SignOff signOff = MessageGenerator::SignOff(client_id, mapreduce::ConnectionType::CLIENT);
        pipe.sendMessage(signOff);
    }
    spdlog::info("Client {} sign off", client_id);
    exit(0);
}


void Client::waitForResponse(){
    mapreduce::MessageType type = pipe.reciveMessageType();
    if(type == mapreduce::MessageType::JOB_RESULT){
        mapreduce::JobResult result;
        pipe >> result;
        spdlog::info("Client {} received result", client_id);
        for(auto& r : result.values()){
            spdlog::info("{} : {}", r.key(), r.value());
        }
        waitForResponse();
    }else if(type == mapreduce::MessageType::SIGN_OFF){
        signOff();
    }else{
        spdlog::error("Client {} received invalid message type ({})", client_id, type);
        if(!pipe){
            spdlog::error("Client {} connection closed", client_id);
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}


void Client::sendJob(Job job){
    mapreduce::JobRequest jobRequest = 
        MessageGenerator::JobRequest(job.type, job.data, -1, -1);
    pipe.sendMessage(jobRequest);
    waitForResponse();
}


int main(){
    asio::io_service ctx;
    asio::ip::tcp::endpoint ep{
        asio::ip::address::from_string("127.0.0.1"), 1500};
    asio::ip::tcp::socket socket(ctx);
    socket.connect(ep);
    Client client(std::move(socket));
    client.signOn();
    std::string filename;
    std::cin >> filename;
    std::ifstream file("/home/autumn/result.txt");
    if(file.good()){
        std::stringstream buffer;
        buffer << file.rdbuf();
        Job job(mapreduce::JobType::LETTER_COUNT
            , buffer.str());
        client.sendJob(job);
    }else{
        spdlog::error("File {} not found", filename);
    } 
}