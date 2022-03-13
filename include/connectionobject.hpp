#ifndef CONNECTIONOBJECT_H
#define CONNECTIONOBJECT_H

class ConnectionObject {
    public:
        int id{-1};
        bool is_available{true};

        virtual ~ConnectionObject() {}
        virtual void sendMessage(
            google::protobuf::Message& message) = 0;

};

typedef std::shared_ptr<ConnectionObject> connection_ptr;

#endif
