#include "../../include/shared.hpp" 

model::OrderBook getOrderBookSnapshot(CommonConfig& config, uint64_t timestamp) {
    storage::Reader reader = storage::Reader();
    auto fileName = storage::getSymbolDirectory(config.rootDir, config.symbol) + "/" + std::to_string(timestamp);
    reader.loadData(fileName);
    auto baseRecords = reader.getRecords();
    auto orders = reader.getOrders(config.symbol);
    model::OrderBook book(baseRecords);
    for(auto order: orders) {
        if(order.timestamp > timestamp) break;
        book.add(order);
    }
    return book;
} 

void mergeStateAndWrite(CommonConfig& config, uint64_t start, uint64_t end, std::vector<model::OrderData>& orderToMerge) {
    storage::Reader reader = storage::Reader();
    std::vector<uint64_t> timePeriods = config.timeIdx->findIndexesInRange(start, end);
    uint64_t curStart = std::min(timePeriods[0], start);

    std::vector<model::OrderData> currentOrders;
    std::vector<model::OrderData> loadedOrders;
    model::OrderBook book = getOrderBookSnapshot(config, curStart);

    size_t idxOrderToMerge = 0;
    size_t idxTimePeriods = 0;

    for(idxOrderToMerge = 0; idxOrderToMerge < orderToMerge.size(); idxOrderToMerge++) {
       
    }

    if(idxOrderToMerge <)

    while(idxNewOrder < currentOrders.size() && idxOldOrder < timePeriods.size()) {
        auto fileName = storage::getSymbolDirectory(config.rootDir, config.symbol) + "/" + std::to_string(timePeriods[idxOldOrder]);
        reader.loadData(fileName);
        auto orders = reader.getOrders(config.symbol);
        storage::deleteDataAt(fileName);

        
    }



}





