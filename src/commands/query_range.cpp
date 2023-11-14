#include "../../include/commands.hpp"

void QueryRangeCommand::execute() {
    if(config.timeIdx->isEmpty()) {
        QueryResult::printEmptyQuery();
        return;
    }
    if(startTime > endTime) {
        QueryResult::printInvalidQuery();
        return;
    }
    storage::Reader reader = storage::Reader();
    auto timeRangeStarts = config.timeIdx->findIndexesCoverRange(startTime,endTime);
    if(timeRangeStarts.empty()) return;

    queryConfig.printReturnFormat();
    uint64_t curTime = startTime;

    for(int i = 0; i < timeRangeStarts.size(); i++) {

        auto fileName = storage::getFileName(config.rootDir, config.symbol, timeRangeStarts[i]);

        reader.loadData(fileName);
        auto baseRecords = reader.getRecords();
        auto orders = reader.getOrders(config.symbol);
        auto lastTrade = reader.getLastTrade();
        model::OrderBook book(baseRecords);

        for(auto order: orders) {
            if(order.timestamp > curTime) {
                QueryResult result(config.symbol, curTime, lastTrade.qty, lastTrade.price, book);
                result.printResult(queryConfig);
                curTime += granularity;
            }
            if(curTime > endTime) break;
            book.add(order);
            swapIfIsTrade(lastTrade, order);
        }

        if(i == timeRangeStarts.size() - 1 && curTime <= endTime) {
            QueryResult result(config.symbol, curTime, lastTrade.qty, lastTrade.price, book);
            result.printResult(queryConfig);
        }
    }
}