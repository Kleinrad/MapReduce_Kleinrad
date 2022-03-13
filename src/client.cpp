#include <spdlog/spdlog.h>
#include <thread>
#include "protoutils.hpp"
#include "job.hpp"
#include "client.h"


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
        waitForResponse();
    }else{
        spdlog::error("Invalid message type");
        throw std::runtime_error("Client::signOn: Invalid Message recived");
    }
}


void Client::signOff() {
    mapreduce::SignOff signOff = MessageGenerator::SignOff(client_id, mapreduce::ConnectionType::CLIENT);
    pipe.sendMessage(signOff);
    spdlog::info("Client {} sign off", client_id);
    exit(0);
}


void Client::waitForResponse(){
    mapreduce::MessageType type = pipe.reciveMessageType();
    if(type == mapreduce::MessageType::JOB_REQUEST){
        mapreduce::JobResult result;
        pipe >> result;
        spdlog::info("Client {} received result {}", client_id, result.data());
        waitForResponse();
    }else if(type == mapreduce::MessageType::SIGN_OFF){
        signOff();
    }else{
        spdlog::error("Client {} received invalid message type ({})", client_id, type);
    }
}


void Client::sendJob(Job job){
    mapreduce::JobRequest jobRequest = 
        MessageGenerator::JobRequest(job.type, job.data, -1, -1);
    pipe.sendMessage(jobRequest);
}


int main(){
    asio::io_service ctx;
    asio::ip::tcp::endpoint ep{
        asio::ip::address::from_string("127.0.0.1"), 1500};
    asio::ip::tcp::socket socket(ctx);
    socket.connect(ep);
    Client client(std::move(socket));
    std::thread t([&](){
        client.signOn();
    });
    std::cin >> std::cin.rdbuf();
    Job job(mapreduce::JobType::LETTER_COUNT, "abcdefghijklmnopqrstuvwxyz");
    client.sendJob(job);
}