#include "../../include/storages.hpp"

void storage::Reader::loadData(const std::string& fileName) {
    std::ifstream handler(fileName, std::ios::in | std::ios::binary);

    storage_model::Metadata metadata;
    
    orders.clear();
    buyRecords.clear();
    sellRecords.clear();

    handler.read((char *)&metadata, sizeof(storage_model::Metadata));

    buyRecords.resize(metadata.buyCnt);
    sellRecords.resize(metadata.sellCnt);
    orders.resize(metadata.orderCnt);

    handler.read((char *)&lastTrade, sizeof(storage_model::LastTradeRecord));
    for(size_t i = 0 ; i < metadata.buyCnt;i++) {
        storage_model::BaseStateRecord record;
        handler.read((char *)&record, sizeof(storage_model::BaseStateRecord));
    }

    for(size_t i = 0 ; i < metadata.sellCnt;i++) {
        storage_model::BaseStateRecord record;
        handler.read((char *)&record, sizeof(storage_model::BaseStateRecord));
    }

    for(size_t i = 0 ; i < metadata.orderCnt;i++) {
        storage_model::OrderRecord order;
        handler.read((char *)&order, sizeof(storage_model::OrderRecord));
    }
}

model::SideRecords  storage::Reader::getRecords() {
    model::SideRecords records;
    for(auto sr: buyRecords) {
        records[model::Side::BUY][sr.price] = sr.qty;
    }

    for(auto sr: sellRecords) {
        records[model::Side::SELL][sr.price] = sr.qty;
    }
    return records;
}
std::vector<model::OrderData>  storage::Reader::getOrders() {

}

storage_model::LastTradeRecord  storage::Reader::getLastTrade() {
    return lastTrade;
}

