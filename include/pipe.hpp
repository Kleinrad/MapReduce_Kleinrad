#include <asio.hpp>
#include <iostream>
#include <string>

class Pipe {
    asio::ip::tcp::iostream* strm;

    public:
        Pipe(std::string host, std::string port){
            strm = new asio::ip::tcp::iostream(host, port);
        }
        Pipe(asio::ip::tcp::socket socket){
            strm = new asio::ip::tcp::iostream(std::move(socket));
        }
        ~Pipe(){
            strm->close();
            delete strm;
        }

        Pipe &operator<<(std::string value){
            *strm << value << std::endl;
            return *this;
        }

        Pipe &operator>>(std::string &value){
            std::getline(*strm, value);
            return *this;
        }
};