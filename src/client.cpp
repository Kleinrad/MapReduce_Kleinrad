#include <iostream>
#include <string>
#include <spdlog/spdlog.h>
#include "pipe.hpp"

int main() {
    asio::ip::tcp::endpoint ep{
        asio::ip::address::from_string("127.0.0.1"), 1500};
    asio::io_service ctx;
    asio::ip::tcp::socket socket{ctx, ep.protocol()};
    socket.connect(ep);
    Pipe pipe(std::move(socket));
    spdlog::set_level(spdlog::level::debug);
    if(pipe){
        std::string line;
        pipe >> line;
        spdlog::info(line);
        pipe << "Hello from client!";
    }
}