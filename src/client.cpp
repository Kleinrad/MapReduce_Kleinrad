#include <iostream>
#include <string>
#include <spdlog/spdlog.h>
#include "pipe.hpp"

int main() {
    Pipe pipe("127.0.0.1", "1500");
    spdlog::set_level(spdlog::level::debug);
    if(pipe){
        std::string line;
        pipe >> line;
        spdlog::info(line);
        pipe << "Hello from client!";
    }
}