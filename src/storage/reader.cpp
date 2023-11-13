#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "../models/storage_model.cpp"
#include "../models/order_book.cpp"
#include "../models/order.cpp"

class Reader {
public: 
    std::vector<storage_model::BaseStateRecord>  buyRecords, sellRecords;
    std::vector<storage_model::OrderRecord> orders;
    storage_model::LastTradeRecord lastTrade;

    void loadData(const std::string& fileName) {
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

    order_book::SideRecords getRecords() {
        order_book::SideRecords records;
        for(auto sr: buyRecords) {
            records[order::Side::BUY][sr.price] = sr.qty;
        }

        for(auto sr: sellRecords) {
            records[order::Side::SELL][sr.price] = sr.qty;
        }
        return records;
    }
    std::vector<order::OrderData> getOrders() {

    }

    storage_model::LastTradeRecord getLastTrade() {
        return lastTrade;
    }


};
