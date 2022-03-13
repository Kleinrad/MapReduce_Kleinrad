#include "clientmanager.h"
#include <spdlog/spdlog.h>

ClientManager::ClientManager(){}


ClientManager::~ClientManager()
{
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