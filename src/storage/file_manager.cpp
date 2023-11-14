#include "../../include/storages.hpp"

std:: string storage::getSymbolDirectory(const std::string& rootDir, const std::string& symbol) {
    return rootDir + "/" + symbol;
}

std:: string storage::getFileName(const std::string& rootDir, const std::string& symbol, uint64_t time) {
    return storage::getSymbolDirectory(rootDir, symbol) + "/" + std::to_string(time) + ".dat";
}

void storage::createSymbolDirectoryIfNotExist(const std::string& rootDir, const std::string& symbol){
    if (!std::filesystem::exists(rootDir)) 
        std::filesystem::create_directory(rootDir);
    auto symbolDir = rootDir + "/" + symbol;
    if (!std::filesystem::exists(symbolDir)) 
                std::filesystem::create_directory(symbolDir);
}
