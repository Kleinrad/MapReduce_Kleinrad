#include <iostream>
#include <string>
#include <spdlog/spdlog.h>
#include "pipe.hpp"

int main() {
    asio::io_context ctx;
    asio::ip::tcp::endpoint ep{asio::ip::tcp::v4(), 1500};
    asio::ip::tcp::acceptor acceptor(ctx, ep);
    asio::ip::tcp::socket socket(ctx);
    spdlog::set_level(spdlog::level::debug);
    while(true){
        acceptor.listen();
        spdlog::info("just before accept");
        acceptor.accept(socket);
        spdlog::info("just after accept");
        Pipe pipe(std::move(socket));
        if(pipe){
            pipe << "Hello from server!";
        }
    }
}