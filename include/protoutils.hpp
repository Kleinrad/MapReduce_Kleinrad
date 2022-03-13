#ifndef PROTOUTILS_H
#define PROTOUTILS_H

#include "Message.pb.h"
#include <spdlog/spdlog.h>
#include <map>

class MessageGenerator{
    public: 
        static mapreduce::Assignment Assignment(int id, mapreduce::ConnectionType type){
            mapreduce::Assignment assignment;
            assignment.set_type(mapreduce::MessageType::ASSIGNMENT);
            assignment.set_id(id);
            assignment.set_connection_type(type);
            return assignment;
        }

        static mapreduce::SignOff SignOff(int id, mapreduce::ConnectionType type){
            mapreduce::SignOff signOff;
            signOff.set_type(mapreduce::MessageType::SIGN_OFF);
            signOff.set_id(id);
            signOff.set_connection_type(type);
            return signOff;
        }

        static mapreduce::Confirm Confirm(int id, mapreduce::ConnectionType type){
            mapreduce::Confirm confirm;
            confirm.set_type(mapreduce::MessageType::CONFIRM);
            confirm.set_id(id);
            confirm.set_connection_type(type);
            return confirm;
        }

        static mapreduce::Authentication Authentication(mapreduce::ConnectionType type){
            mapreduce::Authentication auth;
            auth.set_type(type);
            return auth;
        }
};

#endif