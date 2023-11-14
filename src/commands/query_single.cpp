#include "../../include/commands.hpp"

void QuerySingleCommand::execute()  {
    storage::Reader reader = storage::Reader();
    auto fileName = storage::getSymbolDirectory(config.rootDir, config.symbol) + "/" + std::to_string(timestamp);

    reader.loadData(fileName);
    auto baseRecords = reader.getRecords();
    auto orders = reader.getOrders(config.symbol);
    auto lastTrade = reader.getLastTrade();
    model::OrderBook book(baseRecords);
    for(auto order: orders) {
        if(order.timestamp > timestamp) break;
        book.add(order);
        if(order.category == model::Category::TRADE) {
            lastTrade.price = order.price;
            lastTrade.qty = order.qty;
            lastTrade.timestamp = order.timestamp;
        }
    }

    QueryResult result(config.symbol, timestamp, lastTrade.qty, lastTrade.price, book);
    result.printResult();
}
