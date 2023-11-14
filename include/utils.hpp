#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <stdint.h>
#include <iostream>
#include <vector>
#include "./models.hpp"

constexpr uint64_t PERIOD = 1200000000000;

bool isSamePeriod(uint64_t t1, uint64_t t2) {
    if(t1 < 0 || t2 < 0) return false;
    return abs((long long)(t1 - t2)) <= PERIOD;
}

struct QueryResult {
    model::Symbol symbol;
    uint64_t timestamp;
    uint64_t lastTradeQty;
    double lastTradePrice;
    std::vector<std::pair<double,uint64_t>> topBuys, topSells;

    QueryResult(const model::Symbol& symbol, uint64_t timestamp, uint64_t lastTradeQty, double lastTradePrice, model::OrderBook& book): 
        symbol(symbol), timestamp(timestamp), lastTradeQty(lastTradeQty), lastTradePrice(lastTradePrice) {
            topBuys = book.getTop(model::Side::BUY, 5);
            topSells = book.getTop(model::Side::SELL, 5);
    }

    void printResult(){
        std::cout << symbol << ", " << timestamp << ", ";
    }
};

#endif
