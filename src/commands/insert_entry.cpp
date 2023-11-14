#include "../../include/commands.hpp"


void InsertEntryCommand::execute() {
    std::vector<model::OrderData> orderToInsert{newEntry};
    model::OrderBook book;
    storage_model::LastTradeRecord lastTrade;
    if(config.timeIdx->isEmpty()) {
        writeNewFile(config.rootDir, config.symbol, newEntry.timestamp, book, orderToInsert, lastTrade);
        config.timeIdx->loadIdxFromFile();
    } else {
        mergeStateAndWrite(config,newEntry.timestamp, newEntry.timestamp + 1, orderToInsert);
    }
}


