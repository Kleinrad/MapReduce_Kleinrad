#include <asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <spdlog/spdlog.h>
#include "test.pb.h"

class Pipe {
    asio::ip::tcp::iostream* strm;

    public:
        Pipe(std::string host, std::string port){
            GOOGLE_PROTOBUF_VERIFY_VERSION;
            strm = new asio::ip::tcp::iostream(host, port);
        }
        Pipe(asio::ip::tcp::socket socket){
            GOOGLE_PROTOBUF_VERIFY_VERSION;
            strm = new asio::ip::tcp::iostream(std::move(socket));
        }
        ~Pipe(){
            google::protobuf::ShutdownProtobufLibrary();
            strm->close();
            delete strm;
        }
        
        explicit operator bool()
        {
            return strm->good();
        }

        Pipe &operator<<(std::string value){
            Msg *message = new Msg();
            message->set_text(value);
            message->SerializeToOstream(strm);
            return *this;
        }

        Pipe &operator>>(std::string &value){
            Msg* message = new Msg();
            spdlog::debug("Reading");
            message->ParseFromIstream(strm);
            spdlog::debug("Read: {}", message->text());
            value = message->text();
            return *this;
        }
};