#ifndef CLIENT_H
#define CLIENT_H

#include <asio.hpp>
#include "pipe.hpp"
#include "Message.pb.h"
#include "job.hpp"

class Client{
    int client_id;
    Pipe pipe;
    int lastJobTotal{0};
    std::map<std::string, int> lastJobResult;

    void waitForResponse();
    public:
        bool good;

        Client(asio::ip::tcp::socket socket);
        ~Client();

        void signOff();
        void signOn();
        void sendJob(Job job);

        void printResultsPlain(bool sorted = false);
        void printResultsHistogram(bool sorted = false);
};

#endif