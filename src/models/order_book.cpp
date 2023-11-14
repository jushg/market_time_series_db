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
        update(newOrder.side,newOrder.qty, newOrder.price);
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
    for(auto [k,v]: records[side]) {
        if(v >0 ) {
            ans.push_back({k,v});
            topN--;
        }
        if(topN == 0) return ans;
    }
    return ans;
}

model::SideRecords model::OrderBook::getSideRecords() {
    return records;
}



