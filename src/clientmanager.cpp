#include "clientmanager.h"
#include <spdlog/spdlog.h>

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
    totalConnections--;
    spdlog::info("Client {} sign off", client->id);
    std::lock_guard<std::mutex> lock(mtx);
    clients.erase(client);
}


int ClientManager::generateID(){
    return totalConnections;
}