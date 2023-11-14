#include "../../include/commands.hpp"

void QueryRangeCommand::execute() {

    if(startTime > endTime) {
        std:: cout << "Not Valid"<<std::endl;
        return;
    }
    storage::Reader reader = storage::Reader();
    auto timeRangeStarts = config.timeIdx->findIndexesCoverRange(startTime,endTime);
    if(timeRangeStarts.empty()) return;
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
                result.printResult();
                curTime += granularity;
            }
            if(curTime > endTime) break;
            book.add(order);
            if(order.category == model::Category::TRADE) {
                lastTrade.price = order.price;
                lastTrade.qty = order.qty;
                lastTrade.timestamp = order.timestamp;
            }
        }

        if(i == timeRangeStarts.size() - 1 && curTime <= endTime) {
            QueryResult result(config.symbol, curTime, lastTrade.qty, lastTrade.price, book);
            result.printResult();
        }
    }
}