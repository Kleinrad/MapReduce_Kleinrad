/*
author: Kleinrad Fabian
matnr: i17053
file: messageQueue.hpp
class: 5BHIF
catnr: 07
*/

#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include "Message.pb.h"
#include <condition_variable>
#include <thread>
#include <queue>


struct QueueItem{
    mapreduce::MessageType type;
    mapreduce::JobType jobType;
    int jobId;
    char* dataRaw;
    std::vector<std::pair<std::string, int>> *dataReduce = new std::vector<std::pair<std::string, int>>();
    std::map<std::string, int>* dataResult = new std::map<std::string, int>();

    QueueItem(mapreduce::MessageType type)
        : type(type) {}
};


class MessageQueue
{
    std::queue<QueueItem*> queue;
    std::mutex mtx;
    std::mutex mtx_cond;
    std::condition_variable cv;

  public:

    void push(QueueItem* item){
        std::lock_guard<std::mutex> lock(mtx);
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