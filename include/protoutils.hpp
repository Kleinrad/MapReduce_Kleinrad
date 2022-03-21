#ifndef PROTOUTILS_H
#define PROTOUTILS_H

#include "Message.pb.h"
#include <spdlog/spdlog.h>
#include <map>
#include <set>

class MessageGenerator{
    public: 
        static mapreduce::Assignment Assignment(int id, mapreduce::ConnectionType type){
            mapreduce::Assignment assignment;
            assignment.set_type(mapreduce::MessageType::ASSIGNMENT);
            assignment.set_id(id);
            assignment.set_connection_type(type);
            return assignment;
        }

        static mapreduce::SignOff SignOff(int id, mapreduce::ConnectionType type){
            mapreduce::SignOff signOff;
            signOff.set_type(mapreduce::MessageType::SIGN_OFF);
            signOff.set_id(id);
            signOff.set_connection_type(type);
            return signOff;
        }

        static mapreduce::Confirm Confirm(int id, mapreduce::ConnectionType type){
            mapreduce::Confirm confirm;
            confirm.set_type(mapreduce::MessageType::CONFIRM);
            confirm.set_id(id);
            confirm.set_connection_type(type);
            return confirm;
        }

        static mapreduce::Authentication Authentication(mapreduce::ConnectionType type){
            mapreduce::Authentication auth;
            auth.set_type(mapreduce::MessageType::AUTHENTICATION);
            auth.set_connection_type(type);
            return auth;
        }

        static mapreduce::JobRequest JobRequest(mapreduce::JobType type,
                std::string data, int mappers, int reducers){
            mapreduce::JobRequest jobRequest;
            jobRequest.set_type(mapreduce::MessageType::JOB_REQUEST);
            jobRequest.set_job_type(type);
            jobRequest.set_data(data);
            if(mappers > 0){
                jobRequest.set_num_mappers(mappers);
                jobRequest.set_num_reducers(reducers);
            }if(reducers > 0){
                jobRequest.set_num_reducers(reducers);
            }
            if(mappers <= 0){
                jobRequest.set_num_mappers(-1);
                jobRequest.set_num_reducers(-1);
            }
            return jobRequest;
        }

        static mapreduce::TaskMap TaskMap(mapreduce::JobType type
            , std::string data, int job_id){
            mapreduce::TaskMap taskMap;
            taskMap.set_type(mapreduce::MessageType::TASK_MAP);
            taskMap.set_job_id(job_id);
            taskMap.set_job_type(type);
            taskMap.set_data(data);
            return taskMap;
        }

        static mapreduce::Ping Ping(){
            mapreduce::Ping ping;
            ping.set_type(mapreduce::MessageType::PING);
            return ping;
        }

        static mapreduce::ResultMap ResultMap(
            std::vector<std::pair<std::string, int>> result, int job_id){
            mapreduce::ResultMap resultMap;
            resultMap.set_type(mapreduce::MessageType::RESULT_MAP);
            resultMap.set_job_id(job_id);
            for(auto& pair : result){
                mapreduce::ResultMap::MapValues* v = resultMap.add_values();
                v->set_key(pair.first);
                v->set_value(pair.second);
            }
            return resultMap;
        }

        static mapreduce::TaskReduce TaskReduce(mapreduce::JobType type
            , std::vector<std::pair<std::string, int>> data
            , int job_id){
            mapreduce::TaskReduce taskReduce;
            taskReduce.set_type(mapreduce::MessageType::TASK_REDUCE);
            taskReduce.set_job_id(job_id);
            taskReduce.set_job_type(type);
            mapreduce::TaskReduce::ReduceData* rd = taskReduce.mutable_data();
            for(auto& pair : data){
                mapreduce::TaskReduce::ReduceData::Data* v = rd->add_values();
                v->set_key(pair.first);
                v->set_value(pair.second);
            }
            return taskReduce;
        }

        static mapreduce::ResultReduce ResultReduce(std::map<std::string, int> &result
            , int job_id){
            mapreduce::ResultReduce resultReduce;
            resultReduce.set_type(mapreduce::MessageType::RESULT_REDUCE);
            resultReduce.set_job_id(job_id);
            for(auto& pair : result){
                mapreduce::ResultReduce::ReduceValues* v = resultReduce.add_values();
                v->set_key(pair.first);
                v->set_value(pair.second);
            }
            return resultReduce;
        }

        static mapreduce::JobResult JobResult(int job_id
            , std::map<std::string, int> &result){
            mapreduce::JobResult jobResult;
            jobResult.set_type(mapreduce::MessageType::JOB_RESULT);
            jobResult.set_job_id(job_id);
            for(auto& pair : result){
                mapreduce::JobResult::ResultData* v = jobResult.add_values();
                v->set_key(pair.first);
                v->set_value(pair.second);
            }
            return jobResult;
        }
};

#endif