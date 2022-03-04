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

        Pipe &operator<<(google::protobuf::Message &message){
            spdlog::debug(message.GetDescriptor()->name());
            u_int64_t msg_size{message.ByteSizeLong()};
            asio::write(*socket, asio::buffer(&msg_size, sizeof(msg_size)));
            asio::streambuf buf;
            std::ostream os(&buf);
            message.SerializeToOstream(&os);
            asio::write(*socket, buf);
            return *this;
        }

        Pipe &operator>>(std::string &value){
            /*u_int64_t msg_size;
            socket->receive(asio::buffer(&msg_size, sizeof(msg_size)));
            asio::streambuf buf;
            asio::streambuf::mutable_buffers_type bufs{buf.prepare(msg_size)};
            buf.commit(asio::read(*socket, bufs));
            std::istream is(&buf);
            Msg message;
            message.ParseFromIstream(&is);*/
            value = "test";
            return *this;
        }
};

#endif