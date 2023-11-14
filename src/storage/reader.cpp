#include "../../include/storages.hpp"

void storage::Reader::loadData(const std::string& fileName) {

    if (!std::filesystem::exists(fileName)) {
        std::cout << "File "+fileName + " not existed" <<"\n";
        return;
    }

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

model::SideRecords storage::Reader::getRecords() {
    model::SideRecords records;
    for(auto sr: buyRecords) {
        records[model::Side::BUY][sr.price] = sr.qty;
    }

    for(auto sr: sellRecords) {
        records[model::Side::SELL][sr.price] = sr.qty;
    }
    return records;
}
std::vector<model::OrderData>  storage::Reader::getOrders(std::string& symbol) {
     std::unordered_map<char, model::Category> catergoryMap = {
        {'T', model::Category::TRADE},
        {'N', model::Category::NEW},
        {'C', model::Category::CANCEL}
    };
    std::unordered_map<char, model::Side> sideMap = {
        {'S', model::Side::SELL},
        {'B', model::Side::BUY}
    };

    std::vector<model::OrderData> ans;

    for(auto order: orders) {
        model::OrderData loadedOrder(symbol, order.timestamp, order.id,sideMap[order.side], 
                                                    catergoryMap[order.category], order.qty, order.price);
        ans.push_back(loadedOrder);
    }
    return ans;
}

storage_model::LastTradeRecord  storage::Reader::getLastTrade() {
    return lastTrade;
}

