#include "../../include/client.hpp"

 std::shared_ptr<storage::TimeIndex> EngineData::findIdx(model::Symbol symbol) {
    if (symbolIdxMap.find(symbol) == symbolIdxMap.end()){
        symbolIdxMap[symbol] = std::make_shared<storage::TimeIndex>(symbol, rootDir);
    }
    return symbolIdxMap[symbol];
}

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


void runEngine(EngineData& engineData) {
    bool isRunning = true;
    while (isRunning) {
        std::cout <<"Waiting for user input" <<std::endl;
        std::string cmd;
        std::getline(std::cin, cmd);
        isRunning = parseAndExecute(cmd,engineData);
    }
    std::cout <<"Shutting down the engine" <<std::endl;
}