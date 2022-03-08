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
            return socket->is_open();
        }

        void sendMessage(google::protobuf::Message &message){
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
            while(true){
                try{
                    spdlog::info("wait for message type");
                    socket->receive(asio::buffer(&msgIndex, sizeof(msgIndex)));
                    break;
                }catch(const std::exception& e){
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
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