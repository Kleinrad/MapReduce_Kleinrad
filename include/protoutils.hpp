#ifndef PROTOUTILS_H
#define PROTOUTILS_H

#include "Message.pb.h"
#include <spdlog/spdlog.h>
#include <map>

mapreduce::WorkerAssignment generateWorkerAssignment(int id){
    mapreduce::WorkerAssignment assignment;
    assignment.set_type(mapreduce::MessageType::WORKER_ASSIGNMENT);
    assignment.set_worker_id(id);
    return assignment;
}

mapreduce::WorkerSignOff generateWorkerSignOff(int id){
    mapreduce::WorkerSignOff signOff;
    signOff.set_type(mapreduce::MessageType::WORKER_SIGN_OFF);
    signOff.set_worker_id(id);
    return signOff;
}

mapreduce::Confirm generateConfirm(int id){
    mapreduce::Confirm confirm;
    confirm.set_type(mapreduce::MessageType::CONFIRM);
    confirm.set_worker_id(id);
    return confirm;
}

#endif