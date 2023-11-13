#include <string>
#include <memory>
#include <sstream>
#include "./storages.hpp"
#include "./models.hpp"
#include "./utils.hpp"

class BaseCommand {
public:
    virtual void execute(std::string& rootDir, std::string& symbol, std::shared_ptr<storage::TimeIndex> timeIdx);
};

class InsertEntryCommand: public BaseCommand {
public:
    void execute(std::string& rootDir, std::string& symbol, std::shared_ptr<storage::TimeIndex> timeIdx) override ;
};

class InsertFileCommand: public BaseCommand {
    std::string inputFile;
    model::OrderData parseInputToOrderData(std::string& inputLine);
public:
    InsertFileCommand(std::string&  fileName): inputFile(fileName) {}
    void execute(std::string& rootDir, std::string& symbol, std::shared_ptr<storage::TimeIndex> timeIdx) override ;
};


class QueryRangeCommand: public BaseCommand {
    uint64_t startTime;
    uint64_t endTime;
    uint64_t granularity;
public:
    QueryRangeCommand(uint64_t startTime, uint64_t endTime, uint64_t granularity): 
        startTime(startTime), endTime(endTime), granularity(granularity) {}
    void execute(std::string& rootDir, std::string& symbol, std::shared_ptr<storage::TimeIndex> timeIdx) override ;
};


class QuerySingleCommand: public BaseCommand {
    uint64_t timestamp;
public:
    QuerySingleCommand(uint64_t timestamp): timestamp(timestamp) {}
    void execute(std::string& rootDir, std::string& symbol, std::shared_ptr<storage::TimeIndex> timeIdx) override ;
};