#include "../../include/storages.hpp"

storage::TimeIndex::TimeIndex(std::string& symbol, std::string& rootDir): symbol(symbol), rootDir(rootDir) {
    storage::createSymbolDirectoryIfNotExist(rootDir, symbol);
    idxFilePath = storage::getSymbolDirectory(rootDir, symbol) + "/idx.dat";
    loadIdx();
}

storage::TimeIndex::~TimeIndex(){
    buildIdxFromData();
}

void storage::TimeIndex::loadIdx() {
    if (!std::filesystem::exists(idxFilePath)) {
        loadIdxFromFile();
        return;
    }
    std::ifstream handler(idxFilePath, std::ios::in | std::ios::binary);

    uint64_t idxCnt;
    handler.read((char *)&idxCnt, sizeof(uint64_t));

    for (int i = 0; i < idxCnt; i++) {
        uint64_t idxData;
        handler.read((char *)&idxData, sizeof(uint64_t));
        idxes.insert(idxData);
    }
    handler.close();
}

void storage::TimeIndex::loadIdxFromFile() {
    std::vector<uint64_t> timestamps;
    for (auto const& dir_entry : std::filesystem::directory_iterator{ getSymbolDirectory(rootDir, symbol) }) {
        uint64_t fileStartTime = std::stoll(dir_entry.path().filename());
        timestamps.push_back(fileStartTime);
    }
    idxes.clear();
    idxes = std::set(timestamps.begin(), timestamps.end());
}

void storage::TimeIndex::buildIdxFromData() {
    std::ofstream handler(idxFilePath, std::ios::out | std::ios::binary);
    uint64_t idxCnt = idxes.size();
    handler.write((char *)&idxCnt, sizeof(uint64_t));

    for (auto idxData: idxes) {
        handler.write((char *)&idxData, sizeof(uint64_t));
    }
    handler.close();
}


bool storage::TimeIndex::isEmpty() {
    return idxes.empty();
}

std::vector<uint64_t> storage::TimeIndex::findIndexesCoverRange(const uint64_t startTime, const uint64_t endTime){
    std::vector<uint64_t> ans;
    auto iterStart = idxes.lower_bound(startTime);
    if(iterStart != idxes.begin() && *iterStart != startTime) iterStart--;

    auto iterEnd = idxes.lower_bound(endTime);
    if(iterEnd == idxes.end()) iterEnd--;

    while(iterStart != iterEnd){
        ans.push_back(*iterStart);
        iterStart = next(iterStart);
    }
    ans.push_back(*iterStart);
    return ans;
}

uint64_t storage::TimeIndex::findNearestIndexAfter(const uint64_t time){
    auto iter = idxes.lower_bound(time);
    if(iter == idxes.end()) return -1;
    return *iter;
}

uint64_t storage::TimeIndex::findNearestIndexPrior(const uint64_t time) {
    auto iter = idxes.lower_bound(time);
    if(*iter == time) return time;
    if(iter == idxes.begin())return -1;
    return *prev(iter);
}


