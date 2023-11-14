#ifndef SHARED_HPP
#define SHARED_HPP

#include <string>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <bitset>
#include "./storages.hpp"
#include "./models.hpp"

struct CommonConfig {
    std::string& rootDir;
    std::string& symbol;
    std::shared_ptr<storage::TimeIndex> timeIdx;
};

struct QueryConfig {
    std::unordered_map<model::Side, std::bitset<5>> topOrderNeeded;
    QueryConfig();
    QueryConfig(std::vector<std::string>& fields);
    bool needLastTrade;
    void printReturnFormat();
};

constexpr uint64_t MINUTES = 10; // 10 minutes
constexpr uint64_t PERIOD = 60000000000 * MINUTES; 

constexpr int numOrderToHold = 5;

inline bool isSamePeriod(uint64_t t1, uint64_t t2) {
    if(t1 < 0 || t2 < 0) return false;
    return abs((long long)(t1 - t2)) <= PERIOD;
}

struct QueryResult {
    std::string symbol;
    uint64_t timestamp;
    uint64_t lastTradeQty;
    double lastTradePrice;

    std::unordered_map<model::Side, std::vector<std::pair<double,uint64_t>>> topOrders;

    QueryResult(const model::Symbol& symbol, uint64_t timestamp, uint64_t lastTradeQty, double lastTradePrice, model::OrderBook& book);
    static void printEmptyQuery();
    static void printInvalidQuery();
    void printResult(QueryConfig& config);
};

model::OrderBook getOrderBookSnapshot(CommonConfig& config, uint64_t timestamp);

void mergeStateAndWrite(CommonConfig& config, uint64_t start, uint64_t end, std::vector<model::OrderData>& orderToMerge);
void swapIfIsTrade(storage_model::LastTradeRecord& lastTrade, model::OrderData& order);
void writeNewFile(
    std::string& rootDir, 
    std::string& symbol, 
    uint64_t timestamp, 
    model::OrderBook& book, 
    std::vector<model::OrderData>& orderData, storage_model::LastTradeRecord& lastTrade);

#endif
