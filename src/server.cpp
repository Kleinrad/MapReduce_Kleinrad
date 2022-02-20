#include <asio.hpp>
#include <iostream>
#include <string>
#include <spdlog/spdlog.h>

int main() {
    asio::io_context ctx;
    asio::ip::tcp::endpoint ep{asio::ip::tcp::v4(), 1500};
    asio::ip::tcp::acceptor acceptor(ctx, ep);
    asio::ip::tcp::socket socket(ctx);
    while(true){
        acceptor.listen();
        spdlog::info("just before accept");
        acceptor.accept(socket);
        asio::ip::tcp::iostream strm{std::move(socket)};
        if(strm){
            try{
                strm << "Hello";
                spdlog::info("Accepted connection from");
            }
            catch(const std::exception& e){
                spdlog::error("Sending failed: {}", e.what());
            }
        }else{
            spdlog::error("Connection unsuccessful!");
        }
        strm.close();
    }
}