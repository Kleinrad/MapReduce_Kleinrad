#include <spdlog/spdlog.h>
#include <thread>
#include "pipe.hpp"
#include "workermanager.h"

int main(){
    std::thread worker_manager_thread{WorkerManager()};
    worker_manager_thread.join();
}