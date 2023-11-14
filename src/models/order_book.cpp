#include "../../include/models.hpp"

void model::OrderBook::update (model::Side side, u_int64_t qty, double price) {
    records[side][price] += qty;
}

void model::OrderBook::add(std::vector<model::OrderData>& newOrders) {
    for(auto order: newOrders) add(order);
}

void model::OrderBook::add(model::OrderData newOrder) {
    switch (newOrder.category)
    {
    case model::Category::NEW:
        update(newOrder.side, newOrder.qty, newOrder.price);
        break;
    case model::Category::TRADE:
        update(newOrder.side,(-1) * newOrder.qty, newOrder.price);
        break;

    case model::Category::CANCEL:
        update(newOrder.side,(-1) * newOrder.qty, newOrder.price);
        break;

    default:
        // Add exception
        break;
    }
};

std::vector<std::pair<double,uint64_t>> model::OrderBook::getTop(model::Side side, size_t topN) {
    std::vector<std::pair<double,uint64_t>> ans;
    if(side == model::Side::BUY) {
        for(auto iter = records[side].rbegin(); iter != records[side].rend(); iter++) {
            if(iter->second >0) {
                ans.push_back({iter->first, iter->second});
            }
            if(ans.size() == topN) return ans;
        }
    } else {
        for(auto iter = records[side].begin(); iter != records[side].end(); iter++) {
            if(iter->second >0) {
                ans.push_back({iter->first, iter->second});
            }
            if(ans.size() == topN) return ans;
        }
    }
    return ans;
}

model::SideRecords model::OrderBook::getSideRecords() {
    return records;
}



