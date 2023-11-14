#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_map>
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

    std::shared_ptr<storage::TimeIndex> findIdx(model::Symbol symbol)
    {
        if (symbolIdxMap.find(symbol) == symbolIdxMap.end()){
            symbolIdxMap[symbol] = std::make_shared<storage::TimeIndex>(symbol, rootDir);
        }
        return symbolIdxMap[symbol];
    }
};

EngineData initEngine() {
    std::cout << "Please input a path to the disk storage, or enter 0 to use the default path"<<std::endl;
    std::string rootDir; 
    std::cin >> rootDir;
    std::cin.ignore();
    if(rootDir == "0") rootDir = DEFAULT_STORAGE_NAME;

    if (!std::filesystem::exists(rootDir)) {
        std::cout << "Persistence storage not found, init new storage at local directory "+ rootDir << std::endl;
        std::filesystem::create_directory(rootDir); 
    } else {
        std::cout << "Persistence storage found"<< std::endl;
    }     

    return EngineData(rootDir);
}



QueryConfig getQueryConfig(size_t startFrom, std::vector<std::string>& fields) {
    if(fields[startFrom] == ALL) return QueryConfig();
    std::vector<std::string> dataFields(fields.begin()+startFrom, fields.end());
    return QueryConfig(dataFields);
}


bool runCmd(std::string& cmd, EngineData& engineData) {
    std::stringstream stream(cmd);
    // std::istream_iterator<std::string> begin(stream);
    // std::istream_iterator<std::string> end;
    std::vector<std::string> fields((std::istream_iterator<std::string>(stream)), std::istream_iterator<std::string>());

    if (fields.size() < 1) {
        return true;
    }

    // INSERT <symbol> <epoch> <id> <side:BUY/SELL> <category:NEW/TRADE/CANCEL> <price> <quantity>
    if (fields[0] == INSERT) {
        // if(fields.size() != 8) return {ExitCommand()};
        auto symbol = fields[1];
        auto timestamp = std::stoull(fields[2]);
        auto id = std::stoull(fields[3]);
        auto side = fields[4] == "BUY"? model::Side::BUY : model::Side::SELL;
        auto cat = fields[5] == "NEW"? model::Category::NEW : 
                    fields[5] == "TRADE"? model::Category::TRADE :model::Category::CANCEL;
        auto price =  atof(fields[6].c_str());
        auto qty = std::stoull(fields[7]);
        auto cmd = InsertEntryCommand(
                    model::OrderData(symbol,timestamp,id, side, cat, qty,price),
                    CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)});
        cmd.execute();

    //LOAD <file_path> <symbol>
    } 
    else if(fields[0] == LOAD) {
        auto symbol = fields[1];
        auto cmd = LoadFileCommand(fields[2],CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)});
        cmd.execute();

    } else if(fields[0] == FROM_QUERY_SINGLE) {
        auto symbol = fields[1];

        //FROM <symbol> AT <epoch> QUERY <data>
        if(fields[2] == AT) {
            auto timestamp = std::stoull(fields[3]);

            auto cmd = QuerySingleCommand(
                timestamp,
                CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)},
                getQueryConfig(5,fields)
            );
            cmd.execute();
        // FROM <symbol> RANGE <start> <end> <granularity> QUERY <data>
        } else if(fields[2] == RANGE) {
            auto start = std::stoull(fields[3]);
            auto end = std::stoull(fields[4]);
            auto gra = std::stoull(fields[5]);
            auto cmd = QueryRangeCommand(
                start,
                end, 
                gra,
                CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)},
                getQueryConfig(7,fields)
            );
            cmd.execute();
        }
    } else if(fields[0] == FROM_QUERY_MULTIPLE) {
        std::vector<std::string> symbols;
        int idx = 1;
        while(fields[idx] != AT && fields[idx] != RANGE) symbols.push_back(fields[idx++]);

        // FROM_MULTIPLE <symbol_1> <symbol_2> ... <symbol_n> AT <epoch> QUERY <data>
        if(fields[idx] == AT) {
            auto timestamp = std::stoull(fields[idx+1]);
            for(auto symbol: symbols) {
                auto cmd = QuerySingleCommand(
                            timestamp,
                            CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)},
                            getQueryConfig(idx+3,fields)
                        );
                cmd.execute();
            }

        // FROM_MULTIPLE <symbol_1> <symbol_2> ... <symbol_n> RANGE <start> <end> <granularity> QUERY <data>
        } else if(fields[2] == RANGE) {
            auto start = std::stoull(fields[idx+1]);
            auto end = std::stoull(fields[idx+2]);
            auto gra = std::stoull(fields[idx+3]);
            for(auto symbol: symbols) {
                auto cmd = QueryRangeCommand(
                                start, 
                                end, 
                                gra,
                                CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)},
                                 getQueryConfig(idx+5,fields)
                            );
                cmd.execute();
            }
        }
    } else if(fields[0] == QUIT) return false;
    else if(fields[0] == NUKE) {
        NukeDataCommand cmd;
        cmd.execute(engineData.rootDir);
    }
    return true;
}

void runEngine(EngineData& engineData) {
    bool isRunning = true;
    while (isRunning) {
        std::cout <<"Waiting for user input" <<std::endl;
        std::string cmd;
        std::getline(std::cin, cmd);
        isRunning = runCmd(cmd,engineData);
    }
    std::cout <<"Shutting down the engine" <<std::endl;
}

int main() {
    auto engineData = initEngine();
    runEngine(engineData);
    return 0;
}
