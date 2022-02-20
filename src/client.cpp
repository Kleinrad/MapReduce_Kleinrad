#include <asio.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <spdlog/spdlog.h>

int main() {
    asio::ip::tcp::iostream strm{"127.0.0.1", "1500"};
    if(strm){
        std::string line;
        try{
            std::getline(strm, line);
            spdlog::info(line);
        }
        catch(const std::exception& e){
            spdlog::error("Receiving failed: {}", e.what());
        }
    }else{
        spdlog::error("Could not connect to server!");
    }
    strm.close();
}