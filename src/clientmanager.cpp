/*
author: Kleinrad Fabian
matnr: i17053
file: clientmanager.cpp
class: 5BHIF
catnr: 07
*/

#include "clientmanager.h"
#include <spdlog/spdlog.h>
#include "protoutils.hpp"

int Job::job_counter = 0;

ClientManager::ClientManager(){}


ClientManager::~ClientManager()
{
}


void ClientManager::registerJob(int job_id, int clientId)
{
    std::lock_guard<std::mutex> lock(mtx);
    jobClientMap[job_id] = clientId;
}


void ClientManager::join(connectionPtr client)
{
    totalConnections++;
    spdlog::info("Client {} connected", client->id);
    std::lock_guard<std::mutex> lock(mtx);
    clients.insert(client);
}


void ClientManager::leave(connectionPtr client)
{
    spdlog::info("Client {} sign off", client->id);
    std::lock_guard<std::mutex> lock(mtx);
    clients.erase(client);
}

void ClientManager::sendResult(int job_id, std::map<std::string, int> &result)
{
    std::lock_guard<std::mutex> lock(mtx);
    int clientId = jobClientMap[job_id];
    mapreduce::JobResult job_result = MessageGenerator::JobResult(job_id, result);
    for(auto &client : clients){
        if(client->id == clientId){
            client->sendMessage(job_result);
            break;
        }
    }
    jobClientMap.erase(job_id);
}

int ClientManager::generateID(){
    return totalConnections;
}