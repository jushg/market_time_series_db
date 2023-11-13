#include "../../include/storages.hpp"

void storage::write(std::ofstream &handler,model::OrderBook& book,  std::vector<model::OrderData>& orders) {
    auto records = book.getSideRecords();
    storage_model::Metadata metadata{records[model::Side::BUY].size(), records[model::Side::SELL].size(), orders.size()};
    writeMetadata(handler, metadata);
    writeBaseState(handler, records);
    writeOrder(handler, orders);
}

void storage::writeMetadata(std::ofstream &handler,storage_model::Metadata& metadata ) {
    handler.write((char *)&metadata, sizeof(storage_model::Metadata));
}

void storage::writeBaseState(std::ofstream &handler, model::SideRecords& records) {
    for(auto [price,qty]: records[model::Side::BUY]) {
        storage_model::BaseStateRecord storageData {qty,price};
        handler.write((char *)&storageData, sizeof(storage_model::BaseStateRecord));
    }
    for(auto [price,qty]: records[model::Side::SELL]) {
        storage_model::BaseStateRecord storageData {qty,price};
        handler.write((char *)&storageData, sizeof(storage_model::BaseStateRecord));
    }
}

void storage::writeOrder(std::ofstream &handler, std::vector<model::OrderData>& orders) { 
    std::unordered_map<model::Category, char> catergoryMap = {{model::Category::TRADE, 'T'},{model::Category::NEW, 'N'},{model::Category::CANCEL, 'C'}};
    std::unordered_map<model::Side, char> sideMap = {{model::Side::SELL, 'S'},{model::Side::BUY, 'B'}};

    for(auto order: orders) {
        storage_model::OrderRecord newOrderRecord;
        newOrderRecord.category = catergoryMap[order.category];
        newOrderRecord.side = sideMap[order.side];
        newOrderRecord.price = order.price;
        newOrderRecord.qty = order.qty;
        newOrderRecord.timestamp = order.timestamp;
        handler.write((char *)&newOrderRecord, sizeof(storage_model::OrderRecord));
    }
}


