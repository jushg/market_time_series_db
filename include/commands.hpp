#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <memory>
#include <sstream>
#include "./storages.hpp"
#include "./models.hpp"
#include "./shared.hpp" 

class BaseCommand {
public:
    BaseCommand() = default;
    virtual ~BaseCommand() = default;
    virtual void execute() = 0;
};


class NukeDataCommand: public BaseCommand {
public:
    void execute() override {};
    void execute(std::string& rootDir);
};

class InsertEntryCommand: public BaseCommand {
    model::OrderData newEntry;
    CommonConfig config;
public:
    InsertEntryCommand(model::OrderData &&newEntry, CommonConfig&& config): newEntry(newEntry), config(config) {}
    void execute() override ;
};

class LoadFileCommand: public BaseCommand {
    std::string inputFile;
    model::OrderData parseInputToOrderData(std::string& inputLine);
    CommonConfig config;
public:
    LoadFileCommand(std::string&  fileName, CommonConfig&& config): inputFile(fileName), config(config) {}
    void execute() override ;
};


class QueryRangeCommand: public BaseCommand {
    uint64_t startTime;
    uint64_t endTime;
    uint64_t granularity;
    CommonConfig config;
    QueryConfig queryConfig;
public:
    QueryRangeCommand(uint64_t startTime, uint64_t endTime, uint64_t granularity, CommonConfig&& config, QueryConfig&& queryConfig): 
        startTime(startTime), endTime(endTime), granularity(granularity),  config(config), queryConfig(queryConfig) {}
    void execute() override ;
};


class QuerySingleCommand: public BaseCommand {
    uint64_t timestamp;
    CommonConfig config;
    QueryConfig queryConfig;
public:
    QuerySingleCommand(uint64_t timestamp, CommonConfig&& config,  QueryConfig&& queryConfig): 
        timestamp(timestamp), config(config), queryConfig(queryConfig) {}
    void execute() override ;
};
#endif
