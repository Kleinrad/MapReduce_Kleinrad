#include "pipe.hpp"
#include <thread>
#include <chrono>
#include "protoutils.hpp"

int main(){
    asio::io_service ctx;
    asio::ip::tcp::endpoint ep{
        asio::ip::address::from_string("127.0.0.1"), 1500};
    asio::ip::tcp::socket socket(ctx);
    socket.connect(ep);
    Pipe pipe(std::move(socket));
    std::string line;
    pipe >> line;
    spdlog::info("{}", line);
}