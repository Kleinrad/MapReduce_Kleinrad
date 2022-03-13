#include <spdlog/spdlog.h>
#include "protoutils.hpp"
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