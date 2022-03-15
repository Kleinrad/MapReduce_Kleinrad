#ifndef CONNECTIONOBJECT_H
#define CONNECTIONOBJECT_H

#include <chrono>

class ConnectionObject {
    public:
        int id{-1};
        bool is_available{true};
        std::chrono::time_point<std::chrono::system_clock> last_active;

        virtual ~ConnectionObject() {}
        virtual void sendMessage(
            google::protobuf::Message& message) = 0;
        virtual bool isConnected() = 0;

};

typedef std::shared_ptr<ConnectionObject> connection_ptr;

#endif
