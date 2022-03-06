#include "pipe.hpp"
#include <thread>
#include <chrono>
#include "protoutils.hpp"

Worker::Worker(asio::ip::tcp::socket* socket)

int main(){
    asio::io_service ctx;
    asio::ip::tcp::endpoint ep{
        asio::ip::address::from_string("127.0.0.1"), 1500};
    asio::ip::tcp::socket socket(ctx);
    socket.connect(ep);
    Pipe pipe(std::move(socket));
    if(pipe.reciveMessageType() == mapreduce::MessageType::WORKER_SIGN_OFF){
        mapreduce::WorkerAssignment assignment;
        pipe >> assignment;
        spdlog::info("Worker id: {}", assignment.worker_id());
    }
}