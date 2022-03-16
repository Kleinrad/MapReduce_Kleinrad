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
    bool is_closed{false};

    public:
        Pipe(asio::ip::tcp::socket socket){
            GOOGLE_PROTOBUF_VERIFY_VERSION;
            this->socket = new asio::ip::tcp::socket(std::move(socket));
        }

        ~Pipe(){
            spdlog::debug("Pipe destructor");
            google::protobuf::ShutdownProtobufLibrary();
            spdlog::debug("socket {}", socket != nullptr); 
            socket->close();
            spdlog::debug("socket closed {}", socket != nullptr);
            //delete socket;
            spdlog::debug("Pipe dest ended");
        }
        
        explicit operator bool()
        {
            return !is_closed;
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
            try{
                socket->read_some(asio::buffer(&msgIndex, sizeof(msgIndex)));
            }catch(std::system_error &e){
                spdlog::error("Connection ended", e.what());
                is_closed = true;
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