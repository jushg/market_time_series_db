#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <string>
#include <fstream>
#include <set>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "./storage_models.hpp"
#include "./models.hpp"

namespace storage {
    class TimeIndex {
        std::set<uint64_t> idxes;
        std::string symbol;
        std::string idxFilePath;
        std::string rootDir;
    public:
        TimeIndex(std::string& symbol, std::string& rootDir);
        void loadIdx();
        void reloadIdxFromFile() ;
        void buildIdxFromData() ;
        bool isEmpty() ;
        std::vector<uint64_t> findIndexesInRange(const uint64_t startTime, const uint64_t endTime);
        uint64_t findNearestIndexAfter(const uint64_t time);
        uint64_t findNearestIndexPrior(const uint64_t time) ;
    };

    std:: string getSymbolDirectory(const std::string& rootDir, const std::string& symbol) ;
    void createSymbolDirectoryIfNotExist(const std::string& rootDir, const std::string& symbol);

    class Reader {
    public: 
        std::vector<storage_model::BaseStateRecord>  buyRecords, sellRecords;
        std::vector<storage_model::OrderRecord> orders;
        storage_model::LastTradeRecord lastTrade;

        void loadData(const std::string& fileName);

        model::SideRecords getRecords();
        std::vector<model::OrderData> getOrders(std::string& symbol);
        storage_model::LastTradeRecord getLastTrade();

    };

    void write(std::ofstream &handler,model::OrderBook& book,  std::vector<model::OrderData>& orders, storage_model::LastTradeRecord& lastTrade) ;
    void writeMetadata(std::ofstream &handler,storage_model::Metadata& metadata );
    void writeLastTrade(std::ofstream &handler,storage_model::LastTradeRecord& lastTrade );

    void writeBaseState(std::ofstream &handler, model::SideRecords& records) ;
    void writeOrder(std::ofstream &handler, std::vector<model::OrderData>& orders) ;

    void deleteDataAt(std::string& fileName);

}
#endif


