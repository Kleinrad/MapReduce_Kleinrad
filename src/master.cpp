#include <spdlog/spdlog.h>
#include <thread>
#include "pipe.hpp"
#include "workermanager.h"

int main(){
    asio::io_context ctx;
    asio::ip::tcp::endpoint ep{asio::ip::address_v4(), 1500};
    spdlog::set_level(spdlog::level::debug);
    WorkerManager manager(ctx, ep);
    manager.acceptWorker();
    ctx.run();
}