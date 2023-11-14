#include "../../include/commands.hpp"


void QuerySingleCommand::execute()  {
    if(config.timeIdx->isEmpty()) {
        QueryResult::printEmptyQuery();
        return;
    }
    storage::Reader reader = storage::Reader();
    auto nearestTimestamp = config.timeIdx->findNearestIndexPrior(timestamp);
    auto fileName = storage::getFileName(config.rootDir, config.symbol, nearestTimestamp);
    
    reader.loadData(fileName);
    auto baseRecords = reader.getRecords();
    auto orders = reader.getOrders(config.symbol);
    auto lastTrade = reader.getLastTrade();
    model::OrderBook book(baseRecords);
    for(auto order: orders) {
        if(order.timestamp > timestamp) break;
        book.add(order);
        swapIfIsTrade(lastTrade, order);
    }
    QueryResult result(config.symbol, timestamp, lastTrade.qty, lastTrade.price, book);
    queryConfig.printReturnFormat();
    result.printResult(queryConfig);
}
