#include "../../include/commands.hpp"


void InsertEntryCommand::execute() {
    std::vector<model::OrderData> orderToInsert{newEntry};
    mergeStateAndWrite(config,newEntry.timestamp, newEntry.timestamp + 1, orderToInsert);
}


