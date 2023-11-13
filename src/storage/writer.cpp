#include "string"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include "../models/storage_model.cpp"
#include "../models/order_book.cpp"
#include "../models/order.cpp"


namespace writer {

    void write(std::ofstream &handler,order_book::OrderBook& book,  std::vector<order::OrderData>& orders) {
        auto records = book.getSideRecords();
        storage_model::Metadata metadata{records[order::Side::BUY].size(), records[order::Side::SELL].size(), orders.size()};
        writeMetadata(handler, metadata);
        writeBaseState(handler, records);
        writeOrder(handler, orders);
    }
    
    void writeMetadata(std::ofstream &handler,storage_model::Metadata& metadata ) {
        handler.write((char *)&metadata, sizeof(storage_model::Metadata));
    }

    void writeBaseState(std::ofstream &handler, order_book::SideRecords& records) {
        for(auto [price,qty]: records[order::Side::BUY]) {
            storage_model::BaseStateRecord storageData {qty,price};
            handler.write((char *)&storageData, sizeof(storage_model::BaseStateRecord));
        }
        for(auto [price,qty]: records[order::Side::SELL]) {
            storage_model::BaseStateRecord storageData {qty,price};
            handler.write((char *)&storageData, sizeof(storage_model::BaseStateRecord));
        }
    }

    void writeOrder(std::ofstream &handler, std::vector<order::OrderData>& orders) { 
        std::unordered_map<order::Category, char> catergoryMap = {{order::Category::TRADE, 'T'},{order::Category::NEW, 'N'},{order::Category::CANCEL, 'C'}};
        std::unordered_map<order::Side, char> sideMap = {{order::Side::SELL, 'S'},{order::Side::BUY, 'B'}};

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
};

