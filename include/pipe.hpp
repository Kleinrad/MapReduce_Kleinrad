/*
author: Kleinrad Fabian
matnr: i17053
file: pipe.hpp
class: 5BHIF
catnr: 07
*/

#ifndef PIPE_H
#define PIPE_H

#include <asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <spdlog/spdlog.h>
#include "Message.pb.h"

class Pipe {
    asio::ip::tcp::socket* socket{nullptr};
    std::mutex mtx;
    bool closed{false};

  public:
    Pipe(asio::ip::tcp::socket socket){
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        this->socket = new asio::ip::tcp::socket(std::move(socket));
    }

    ~Pipe(){
        google::protobuf::ShutdownProtobufLibrary();
        socket->close();
        delete socket;
    }
    
    explicit operator bool()
    {
        return !closed;
    }


    void close(){
        closed = true;
        if(socket)
            socket->close();
    }


    void sendMessage(google::protobuf::Message &message){
        std::lock_guard<std::mutex> lock(mtx);
        u_int8_t msgIndex = message.GetDescriptor()->index();
        u_int64_t msgSize{message.ByteSizeLong()};
        asio::write(*socket, asio::buffer(&msgIndex, sizeof(msgIndex)));
        asio::write(*socket, asio::buffer(&msgSize, sizeof(msgSize)));

        asio::streambuf buf;
        std::ostream os(&buf);
        
        message.SerializeToOstream(&os);
        asio::write(*socket, buf);
    }

    mapreduce::MessageType reciveMessageType(){
        u_int8_t msgIndex;
        try{
            socket->read_some(asio::buffer(&msgIndex, sizeof(msgIndex)));
        }catch(std::system_error &e){
            spdlog::error("Connection ended", e.what());
            closed = true;
        }
        return static_cast<mapreduce::MessageType>(msgIndex);
    }

    Pipe &operator>>(google::protobuf::Message &message){
        u_int64_t msgSize;
        socket->receive(asio::buffer(&msgSize, sizeof(msgSize)));

        asio::streambuf buf;
        asio::streambuf::mutable_buffers_type bufs{buf.prepare(msgSize)};

        buf.commit(asio::read(*socket, bufs));
        std::istream is(&buf);

        message.ParseFromIstream(&is);
        return *this;
    }
};

#endif