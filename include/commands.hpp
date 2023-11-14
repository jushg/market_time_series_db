#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <memory>
#include <sstream>
#include "./storages.hpp"
#include "./models.hpp"
#include "./utils.hpp"
#include "./shared.hpp" 

class BaseCommand {
public:
    virtual void execute();
};

class InsertEntryCommand: public BaseCommand {
    model::OrderData newEntry;
public:
    CommonConfig config;
    InsertEntryCommand(model::OrderData &&newEntry, CommonConfig&& config): newEntry(newEntry), config(config) {}
    void execute() override ;
};

class LoadFileCommand: public BaseCommand {
    std::string inputFile;
    model::OrderData parseInputToOrderData(std::string& inputLine);
public:
    CommonConfig config;
    LoadFileCommand(std::string&  fileName, CommonConfig&& config): inputFile(fileName), config(config) {}
    void execute() override ;
};


class QueryRangeCommand: public BaseCommand {
    uint64_t startTime;
    uint64_t endTime;
    uint64_t granularity;
public:
    CommonConfig config;
    QueryRangeCommand(uint64_t startTime, uint64_t endTime, uint64_t granularity, CommonConfig&& config): 
        startTime(startTime), endTime(endTime), granularity(granularity),  config(config) {}
    void execute() override ;
};


class QuerySingleCommand: public BaseCommand {
    uint64_t timestamp;
public:
    CommonConfig config;
    QuerySingleCommand(uint64_t timestamp, CommonConfig&& config): timestamp(timestamp), config(config) {}
    void execute() override ;
};

#endif