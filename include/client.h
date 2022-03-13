#ifndef CLIENT_H
#define CLIENT_H

#include <asio.hpp>
#include "pipe.hpp"

class Client{
    int client_id;
    Pipe pipe;

    void waitForResponse();
    public:
        Client(asio::ip::tcp::socket socket);
        ~Client();

        void signOff();
        void signOn();
};

#endif