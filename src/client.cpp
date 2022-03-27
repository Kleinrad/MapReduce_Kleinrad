#include <spdlog/spdlog.h>
#include <thread>
#include "protoutils.hpp"
#include "job.hpp"
#include "client.h"
#include <fstream>
#include "CLI11.hpp"

int Job::job_counter = 0;

Client::Client(asio::ip::tcp::socket socket):
    pipe(Pipe(std::move(socket))) {}


Client::~Client() {
    signOff();
}


void Client::signOn(){
    mapreduce::Authentication auth = MessageGenerator::Authentication(
        mapreduce::ConnectionType::CLIENT);
    pipe.sendMessage(auth);
    if(pipe.reciveMessageType() == mapreduce::MessageType::ASSIGNMENT){
        mapreduce::Assignment assignment;
        pipe >> assignment;
        client_id = assignment.id();
        mapreduce::Confirm confirm = MessageGenerator::Confirm(client_id
            , mapreduce::ConnectionType::CLIENT);
        pipe.sendMessage(confirm);
        good = true;
        spdlog::info("Client {} sign on", client_id);
    }else{
        spdlog::error("Invalid message type");
        throw std::runtime_error("Client::signOn: Invalid Message recived");
    }
}


void Client::signOff() {
    if(pipe){
        mapreduce::SignOff signOff = MessageGenerator::SignOff(client_id, mapreduce::ConnectionType::CLIENT);
        pipe.sendMessage(signOff);
    }
    good = false;
    spdlog::info("Client {} sign off", client_id);
    exit(0);
}


void Client::waitForResponse(){
    mapreduce::MessageType type = pipe.reciveMessageType();
    if(type == mapreduce::MessageType::JOB_RESULT){
        mapreduce::JobResult result;
        pipe >> result;
        lastJobResult.clear();
        for(auto& r : result.values()){
            lastJobResult[r.key()] = r.value();
        }
    }else if(type == mapreduce::MessageType::SIGN_OFF){
        signOff();
    }else{
        //spdlog::error("Client {} received invalid message type ({})", client_id, type);
        if(!pipe){
            good = false;
            spdlog::error("Connection closed");
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}


void Client::sendJob(Job job){
    mapreduce::JobRequest jobRequest = 
        MessageGenerator::JobRequest(job.type, job.data, -1, -1);
    lastJobTotal = job.data.size();
    pipe.sendMessage(jobRequest);
    waitForResponse();
}


void Client::printResultsPlain(bool sorted){
    if(lastJobResult.size() == 0){
        spdlog::error("No results to print");
        return;
    }
    std::cout << "\nLast job results: " << std::endl;
    std::cout << "Total characters: " << lastJobTotal << "\n" << std::endl;
    if(sorted){
        std::vector<std::pair<std::string, int>> sortedResult;
        std::copy(lastJobResult.begin(), lastJobResult.end(), std::back_inserter(sortedResult));
        std::sort(sortedResult.begin(), sortedResult.end(),
            [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b){
                return a.second > b.second;
            });
        for(auto& r : sortedResult){
            std::cout << std::setw(15) << r.first << ": " << r.second <<
            " [" << (std::round(((double)r.second/lastJobTotal)*1000)/10) << "%]" << std::endl;
        }
    }else{
        for(auto& r : lastJobResult){
            std::cout << std::setw(15) << r.first << ": " << r.second <<
            " [" << (std::round(((double)r.second/lastJobTotal)*1000)/10) << "%]"  << std::endl;
        }
    }
}

void Client::printResultsHistogram(bool sorted){
    if(lastJobResult.size() == 0){
        spdlog::error("No results to print");
        return;
    }
    std::cout << "\nLast job results: " << std::endl;
    std::vector<std::pair<std::string, int>> sortedResult;
    std::copy(lastJobResult.begin(), lastJobResult.end(), std::back_inserter(sortedResult));
    std::sort(sortedResult.begin(), sortedResult.end(),
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b){
            return a.second > b.second;
        });
    int max = sortedResult.begin()->second;
    if(sorted){
        for(auto& r : sortedResult){
            std::cout << std::setw(15) << r.first << ": ";
            for(int i = 0; i < (std::round(((double)r.second/max)*50)); i++){
                std::cout << "|";
            }
            std::cout << std::endl;
        }
    }else{
        for(auto& r : lastJobResult){
            std::cout << std::setw(15) << r.first << ": ";
            for(int i = 0; i < (std::round(((double)r.second/max)*50)); i++){
                std::cout << "|";
            }
            std::cout << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    CLI::App app{"MapReduce Client"};
    
    std::string ipAddress{"127.0.0.1"};
    asio::ip::port_type port{1500};
    app.add_option("<ip>", ipAddress, "server ip address");
    app.add_option("<port>", port, "port for connection");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    asio::io_service ctx;
    asio::ip::address addr;
    try{
        addr = asio::ip::address::from_string(ipAddress);
    }catch(std::exception& e){
        spdlog::error("Invalid ip address");
        return 1;
    }
    asio::ip::tcp::endpoint ep{
        addr, port};
    asio::ip::tcp::socket socket(ctx);
    spdlog::set_level(spdlog::level::debug);
    try{
        socket.connect(ep);
    } catch(const std::exception& e){
        spdlog::error("Connection failed");
        return 1;
    }
    Client client(std::move(socket));
    client.signOn();
    std::cout << "Connected to server [" << ipAddress << ":" << port << "]\n" << std::endl;

    std::string input;
    while (client.good)
    {
        std::cout << "# " << std::flush;
        std::getline(std::cin, input);
        if(input == "")
            continue;
        if (input == "quit" || input == "q")
            break;
        if (input == "help" || input == "h"){
            std::cout << "\nAvailable commands:" << std::endl;
            std::cout << "- quit, q" << std::endl;
            std::cout << "- help, h" << std::endl;
            std::cout << "- send <jobType> [-f] <data>" << std::endl;
            std::cout << "- print [-s] <printType>" << std::endl;
            std::cout << "\nAvailable jobTypes:" << std::endl;
            std::cout << "(0) character counting" << std::endl;
            std::cout << "(1) word counting" << std::endl;
            std::cout << "\nAvailable printTypes:" << std::endl;
            std::cout << "(0) print last job result" << std::endl;
            std::cout << "(1) print last job result as histogram" << std::endl;
            std::cout << "\nOptional arguments:" << std::endl;
            std::cout << "-f: read data from file" << std::endl;
            std::cout << "-s: sort results\n" << std::endl;
            continue;
        }
        if (input.find("send") == 0){
            if(input.size() < 6){
                spdlog::error("Invalid parameters");
                continue;
            }
            try{
                int type = std::stoi(input.substr(5));
                if(type > 1 || type < 0){
                    spdlog::error("Invalid type");
                    continue;
                }
                std::string data;
                if(input.find("-f") != std::string::npos){
                    if(input.size() < input.find("-f") + 3){
                        spdlog::error("Invalid parameters");
                        continue;
                    }
                    std::ifstream file(input.substr(input.find("-f") + 3));
                    if(!file.is_open()){
                        spdlog::error("File not found");
                        continue;
                    }
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    data = buffer.str();
                }else{
                    if(input.size() < 8){
                        spdlog::error("Invalid data");
                        continue;
                    }
                    data = input.substr(7);
                }
                if(data.size() <= 1){
                    spdlog::error("Invalid data");
                    continue;
                }
                mapreduce::JobType jobType = mapreduce::JobType(type);
                Job job(jobType, data);
                std::cout << "\nJob sent, waiting for reply..." << std::endl;
                client.sendJob(job);
                std::cout << "Job finished" << std::endl;
                continue;
            }catch(std::invalid_argument& e){
                spdlog::error("Invalid type");
                continue;
            }
        }
        if (input.find("print") == 0){
            if(input.size() < 7){
                spdlog::error("Invalid parameters");
                continue;
            }
            try{
                bool sorted = false;
                int type;
                if(input.find("-s") != std::string::npos){
                    if(input.size() < input.find("-s") + 3){
                        spdlog::error("Invalid parameters");
                        continue;
                    }
                    type = std::stoi(input.substr(input.find("-s") + 3));
                    sorted = true;
                }else{
                    type = std::stoi(input.substr(6));
                }
                if(type > 1 || type < 0){
                    spdlog::error("Invalid type");
                    continue;
                }
                if(type == 0){
                    client.printResultsPlain(sorted);
                }else if(type == 1){
                    client.printResultsHistogram(sorted);
                }
                continue;
            }catch(std::invalid_argument& e){
                spdlog::error("Invalid type");
                continue;
            }
        }
        spdlog::error("Invalid command, type 'help' or 'h' for help");
    }
    std::cout << "\nDisconnected from server" << std::endl;
}