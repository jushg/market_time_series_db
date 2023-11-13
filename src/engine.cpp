#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_map>
#include "../include/storages.hpp"
#include "../include/models.hpp"
#include "../include/commands.hpp"


static const std::string SELECT = "SELECT";
static const std::string MULTIPLE = "MULTIPLE";
static const std::string INSERT = "INSERT";
static const std::string FILE = "FILE";


class EngineData {
    std::string rootDir;
    std::unordered_map<model::Symbol, std::shared_ptr<storage::TimeIndex> > symbolIdxMap;

public:
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
    std::cout << "Please input a path to the disk storage"<<std::endl;
    std::string rootDir; 
    std::cin >> rootDir;

    if (!std::filesystem::exists(rootDir)) {
        std::cout << "Persistence storage not found, init new storage"<< std::endl;
        std::filesystem::create_directory(rootDir); 
    } else {
        std::cout << "Persistence storage found"<< std::endl;
    }     

    return EngineData(rootDir);
}


std::vector<BaseCommand> parseCommand(std::string& cmd) {
    std::stringstream stream(cmd);
    std::istream_iterator<std::string> begin(stream);
    std::istream_iterator<std::string> end;
    std::vector<std::string> fields((std::istream_iterator<std::string>(stream)), std::istream_iterator<std::string>());

    if (fields.size() < 1) {
        return {BaseCommand()};
    } 

    

    
}



void runEngine(EngineData& engineData) {
    bool isRunning = true;
    while (isRunning) {
        std::string cmd;
        std::cin >> cmd;
    }
    
}

int main() {
    auto engineData = initEngine();
    runEngine(engineData);
    return 0;
}