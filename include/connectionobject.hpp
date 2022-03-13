#ifndef CONNECTIONOBJECT_H
#define CONNECTIONOBJECT_H

class ConnectionObject {
    
    public:
        int id{-1};
        bool is_available;

        ConnectionObject() {};
        ConnectionObject(int id) 
        : id(id), is_available(true) {};

};

typedef std::shared_ptr<ConnectionObject> connection_ptr;

#endif
