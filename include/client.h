/*
author: Kleinrad Fabian
matnr: i17053
file: client.h
class: 5BHIF
catnr: 07
*/

#ifndef CLIENT_H
#define CLIENT_H

#include <asio.hpp>
#include "pipe.hpp"
#include "Message.pb.h"
#include "job.hpp"

class Client{
    int clientId;
    Pipe pipe;
    int lastJobTotal{0};
    std::map<std::string, int> lastJobResult;

    void waitForResponse();
  
  public:
    Client(asio::ip::tcp::socket socket);
    ~Client();

    void signOff();
    void signOn();
    void sendJob(Job job);

    void printResultsPlain(bool sorted = false);
    void printResultsHistogram(bool sorted = false);

    bool good;
};

#endif