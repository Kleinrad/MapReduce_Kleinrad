/*
author: Kleinrad Fabian
matnr: i17053
file: connectionobject.hpp
class: 5BHIF
catnr: 07
*/

#ifndef CONNECTIONOBJECT_H
#define CONNECTIONOBJECT_H

#include <chrono>

class ConnectionObject {
  public:
    int id{-1};
    bool isAvailable{true};
    std::chrono::time_point<std::chrono::system_clock> lastActive;

    virtual ~ConnectionObject() {}
    virtual void sendMessage(
        google::protobuf::Message& message) = 0;
    virtual bool isConnected() = 0;
    virtual void closeConnection() = 0;

};

typedef std::shared_ptr<ConnectionObject> connectionPtr;

#endif
