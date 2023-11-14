#include "../../include/shared.hpp" 

model::OrderBook getOrderBookSnapshot(CommonConfig& config, uint64_t timestamp) {
    if(config.timeIdx->isEmpty() || config.timeIdx->findNearestIndexPrior(timestamp) == -1) 
        return model::OrderBook();
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
    std::vector<uint64_t> timePeriods = config.timeIdx->findIndexesCoverRange(start, end);

    std::vector<model::OrderData> currentOrders;
    model::OrderBook book = getOrderBookSnapshot(config, timePeriods[0]);

    storage_model::LastTradeRecord lastTrade;

    size_t idxOrderToMerge = 0;

    for(auto startTime: timePeriods) {
        auto fileName = storage::getSymbolDirectory(config.rootDir, config.symbol) + "/" + std::to_string(startTime);
        reader.loadData(fileName);
        auto loadedOrders = reader.getOrders(config.symbol);
        storage::deleteDataAt(fileName);

        for(auto loadedOrder: loadedOrders) {
            if(!currentOrders.empty() && !isSamePeriod(loadedOrder.timestamp, currentOrders.front().timestamp)){
                writeNewFile(config.rootDir, config.symbol, currentOrders.front().timestamp, book, currentOrders, lastTrade);
                book.add(currentOrders);
                currentOrders.clear();
            }

            while(idxOrderToMerge < orderToMerge.size() && orderToMerge[idxOrderToMerge].timestamp < loadedOrder.timestamp) {
                if(orderToMerge[idxOrderToMerge].category == model::Category::TRADE) 
                    lastTrade = storage_model::LastTradeRecord(orderToMerge[idxOrderToMerge]);
                currentOrders.push_back(orderToMerge[idxOrderToMerge++]);
            }
            if(loadedOrder.category == model::Category::TRADE) lastTrade = storage_model::LastTradeRecord(loadedOrder);
            currentOrders.push_back(loadedOrder);
        }
    }

    if(idxOrderToMerge < orderToMerge.size() - 1 || !currentOrders.empty()) {
        while (idxOrderToMerge < orderToMerge.size()) {
            if(orderToMerge[idxOrderToMerge].category == model::Category::TRADE) 
                    lastTrade = storage_model::LastTradeRecord(orderToMerge[idxOrderToMerge]);
            currentOrders.push_back(orderToMerge[idxOrderToMerge++]);
        }
        writeNewFile(config.rootDir, config.symbol, currentOrders.front().timestamp, book, currentOrders, lastTrade);
       
    }
    config.timeIdx->loadIdxFromFile();
}

void writeNewFile( std::string& rootDir, std::string& symbol,  uint64_t startTime, 
    model::OrderBook& book, std::vector<model::OrderData>& orderData, storage_model::LastTradeRecord& lastTrade) {
    auto newFile = storage::getFileName(rootDir, symbol, startTime);
    std::ofstream handler(newFile, std::ios::out | std::ios::binary);
    storage::write(handler, book, orderData, lastTrade);
    handler.close();
}






