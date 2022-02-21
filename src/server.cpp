#include <iostream>
#include <string>
#include <spdlog/spdlog.h>
#include "pipe.hpp"

int main() {
    asio::io_context ctx;
    asio::ip::tcp::endpoint ep{asio::ip::tcp::v4(), 1500};
    asio::ip::tcp::acceptor acceptor(ctx, ep);
    asio::ip::tcp::socket socket(ctx);
    while(true){
        acceptor.listen();
        spdlog::info("just before accept");
        acceptor.accept(socket);
        Pipe pipe(std::move(socket));
        std::string line;
        pipe >> line;
        spdlog::info(line);
        pipe << "Hello from server!";
    }
}