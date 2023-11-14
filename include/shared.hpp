#ifndef SHARED_HPP
#define SHARED_HPP

#include "./storages.hpp"
#include "./models.hpp"


struct CommonConfig {
    std::string& rootDir;
    std::string& symbol;
    std::shared_ptr<storage::TimeIndex> timeIdx;
};


model::OrderBook getOrderBookSnapshot(CommonConfig& config, uint64_t timestamp);
void mergeStateAndWrite(CommonConfig& config, uint64_t start, uint64_t end, std::vector<model::OrderData>& orderToMerge);

#endif
