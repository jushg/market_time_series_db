#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include "../models/order_book.cpp"
#include "../models/order.cpp"
#include "../storage/reader.cpp"
#include "../utils/utils.cpp"

struct QueryResult {
    order::Symbol symbol;
    uint64_t timestamp;
    uint64_t lastTradeQty;
    double lastTradePrice;
    std::vector<std::pair<double,uint64_t>> topBuys, topSells;

    QueryResult(const order::Symbol& symbol, uint64_t timestamp, uint64_t lastTradeQty, double lastTradePrice, order_book::OrderBook& book): 
        symbol(symbol), timestamp(timestamp), lastTradeQty(lastTradeQty), lastTradePrice(lastTradePrice) {
            topBuys = book.getTop(order::Side::BUY, 5);
            topSells = book.getTop(order::Side::SELL, 5);
    }

    void printResult(){
        std::cout << symbol << ", " << timestamp << ", ";
    }
};



void queryRange(const order::Symbol& symbol, const uint64_t startTime, const uint64_t endTime, const uint64_t granularity) {
    Reader reader = Reader();
    auto timeRangeStarts = findIndexesInRange(startTime,endTime);
    if(timeRangeStarts.empty()) return;
    uint64_t curTime = startTime;

    for(int i = 0; i < timeRangeStarts.size(); i++) {
        reader.loadData(getFileName(timeRangeStarts[i],symbol));
        auto baseRecords = reader.getRecords();
        auto orders = reader.getOrders();
        auto lastTrade = reader.getLastTrade();
        order_book::OrderBook book(baseRecords);


        for(auto order: orders) {
            if(order.timestamp > curTime) {
                QueryResult result(symbol, curTime, lastTrade.qty, lastTrade.price, book);
                result.printResult();
                curTime += granularity;
            }
            if(curTime > endTime) break;
            book.add(order);
            if(order.category == order::Category::TRADE) {
                lastTrade.price = order.price;
                lastTrade.qty = order.qty;
                lastTrade.timestamp = order.timestamp;
            }
        }

        if(i == timeRangeStarts.size() - 1 && curTime <= endTime) {
            QueryResult result(symbol, curTime, lastTrade.qty, lastTrade.price, book);
            result.printResult();
        }
    }

}

void queryTimestamp(const order::Symbol& symbol, const uint64_t time) {
    Reader reader = Reader();
    reader.loadData(getFileName(findNearestIndexPrior(time),symbol));
    auto baseRecords = reader.getRecords();
    auto orders = reader.getOrders();
    auto lastTrade = reader.getLastTrade();
    order_book::OrderBook book(baseRecords);
    for(auto order: orders) {
        if(order.timestamp > time) break;
        book.add(order);
        if(order.category == order::Category::TRADE) {
            lastTrade.price = order.price;
            lastTrade.qty = order.qty;
            lastTrade.timestamp = order.timestamp;
        }
    }

    QueryResult result(symbol, time, lastTrade.qty, lastTrade.price, book);
    result.printResult();
}

