#include "../../include/commands.hpp"


void InsertEntryCommand::execute() {
    std::vector<model::OrderData> orderToInsert{newEntry};
    model::OrderBook book;
    storage_model::LastTradeRecord lastTrade;
    if(config.timeIdx->isEmpty()) {
        writeNewFile(config.rootDir, config.symbol, newEntry.timestamp, book, orderToInsert, lastTrade);
        config.timeIdx->loadIdxFromFile();
    } else {
        auto nearestPrior = config.timeIdx->findNearestIndexPrior(newEntry.timestamp);
        if(nearestPrior != static_cast<uint64_t>(-1) && isSamePeriod(nearestPrior, newEntry.timestamp)) {
            mergeStateAndWrite(config, newEntry.timestamp, newEntry.timestamp + 1, orderToInsert);
        } else {
            book = getOrderBookSnapshot(config, newEntry.timestamp);
            writeNewFile(config.rootDir, config.symbol, newEntry.timestamp, book, orderToInsert, lastTrade);
            config.timeIdx->loadIdxFromFile();
        }
    }
}


