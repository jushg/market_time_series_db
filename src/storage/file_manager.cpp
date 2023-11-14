#include "../../include/storages.hpp"

std:: string storage::getSymbolDirectory(const std::string& rootDir, const std::string& symbol) {
    return rootDir + "/" + symbol;
}

void storage::createSymbolDirectoryIfNotExist(const std::string& rootDir, const std::string& symbol){
    if (!std::filesystem::exists(rootDir)) 
        std::filesystem::create_directory(rootDir);
    auto symbolDir = rootDir + "/" + symbol;
    if (!std::filesystem::exists(symbolDir)) 
                std::filesystem::create_directory(symbolDir);
}
