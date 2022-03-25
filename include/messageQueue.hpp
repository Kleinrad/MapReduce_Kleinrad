#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include "Message.pb.h"
#include <condition_variable>
#include <thread>
#include <queue>


struct QueueItem{
    mapreduce::MessageType type;
    google::protobuf::Message* message;
};


class MessageQueue
{
    std::queue<QueueItem*> queue;
    std::mutex mtx;
    std::mutex mtx_cond;
    std::condition_variable cv;

public:
    MessageQueue();
    ~MessageQueue();
    void push(google::protobuf::Message& message){
        std::lock_guard<std::mutex> lock(mtx);
        QueueItem* item = new QueueItem();
        item->type = static_cast<mapreduce::MessageType>(message.GetDescriptor()->index());
        item->message = &message;
        queue.push(item);
        cv.notify_one();
    }
    
    QueueItem* pop(){
        std::unique_lock<std::mutex> lock(mtx_cond);
        cv.wait(lock, [this]{return !queue.empty();});
        QueueItem* item = queue.front();
        queue.pop();
        return item;
    }

    bool isEmpty();
};


#endif