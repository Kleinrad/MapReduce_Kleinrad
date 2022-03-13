#ifndef PROTOUTILS_H
#define PROTOUTILS_H

#include "Message.pb.h"
#include <spdlog/spdlog.h>
#include <map>

class MessageGenerator{
    public: 
        static mapreduce::WorkerAssignment WorkerAssignment(int id){
            mapreduce::WorkerAssignment assignment;
            assignment.set_type(mapreduce::MessageType::WORKER_ASSIGNMENT);
            assignment.set_worker_id(id);
            return assignment;
        }

        static mapreduce::WorkerSignOff WorkerSignOff(int id){
            mapreduce::WorkerSignOff signOff;
            signOff.set_type(mapreduce::MessageType::WORKER_SIGN_OFF);
            signOff.set_worker_id(id);
            return signOff;
        }

        static mapreduce::Confirm Confirm(int id, mapreduce::ConnectionType type){
            mapreduce::Confirm confirm;
            confirm.set_type(mapreduce::MessageType::CONFIRM);
            confirm.set_worker_id(id);
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