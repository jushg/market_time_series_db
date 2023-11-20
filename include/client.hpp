#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../include/storages.hpp"
#include "../include/models.hpp"
#include "../include/commands.hpp"


static const std::string LOAD = "LOAD";
static const std::string INSERT = "INSERT";
static const std::string FROM_QUERY_SINGLE = "FROM";
static const std::string FROM_QUERY_MULTIPLE = "FROM_MULTIPLE";
static const std::string RANGE = "RANGE";
static const std::string AT = "AT";
static const std::string QUERY_FIELD = "QUERY";
static const std::string ALL = "ALL";
static const std::string QUIT = "QUIT";
static const std::string NUKE = "NUKE_DB";


static const std::string DEFAULT_STORAGE_NAME = "data_engine_persistence_storage";

class EngineData {
    std::unordered_map<model::Symbol, std::shared_ptr<storage::TimeIndex> > symbolIdxMap;

public:
    std::string rootDir;
    EngineData(std::string& rootDir): rootDir(rootDir) {}

    std::shared_ptr<storage::TimeIndex> findIdx(model::Symbol symbol);
};

EngineData initEngine();
void runEngine(EngineData& engineData);

QueryConfig getQueryConfig(size_t startFrom, std::vector<std::string>& fields);
bool parseAndExecute(std::string& cmd, EngineData& engineData);

#endif