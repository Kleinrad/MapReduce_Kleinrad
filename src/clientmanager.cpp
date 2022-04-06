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


void ClientManager::registerJob(int job_id, int client_id)
{
    std::lock_guard<std::mutex> lock(mtx);
    job_client_map[job_id] = client_id;
}


void ClientManager::join(connection_ptr client)
{
    totalConnections++;
    spdlog::info("Client {} connected", client->id);
    std::lock_guard<std::mutex> lock(mtx);
    clients.insert(client);
}


void ClientManager::leave(connection_ptr client)
{
    spdlog::info("Client {} sign off", client->id);
    std::lock_guard<std::mutex> lock(mtx);
    clients.erase(client);
}

void ClientManager::sendResult(int job_id, std::map<std::string, int> &result)
{
    std::lock_guard<std::mutex> lock(mtx);
    int client_id = job_client_map[job_id];
    mapreduce::JobResult job_result = MessageGenerator::JobResult(job_id, result);
    for(auto &client : clients){
        if(client->id == client_id){
            client->sendMessage(job_result);
            break;
        }
    }
    job_client_map.erase(job_id);
}

int ClientManager::generateID(){
    return totalConnections;
}