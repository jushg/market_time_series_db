#ifndef MODEL_HPP
#define MODEL_HPP

#include <unordered_map>
#include <map>
#include <string>
#include <stdint.h>
#include <queue>
#include <vector>
#include <algorithm>
#include <utility>

namespace model {
    enum Side {SELL, BUY };
    
    static const Side AllSide[] = { Side::BUY, Side::SELL };

    enum Category {TRADE, NEW, CANCEL};

    static const Category AllCategory[] = { Category::TRADE, Category::NEW, Category::CANCEL };

    typedef std::string Symbol;

    struct OrderData {
        Symbol symbol;
        uint64_t timestamp;
        uint64_t id;
        Side side;
        Category category;
        uint64_t qty;
        double price;

        OrderData(Symbol symbol,
            uint64_t timestamp,
            uint64_t id,
            Side side,
            Category category,
            uint32_t qty,
            double price)
            : symbol(symbol),timestamp(timestamp), id(id), side(side), category(category), qty(qty), price(price) {}
    };

    typedef std::map<double, uint64_t> OrderRecord;

    typedef std::unordered_map<Side, OrderRecord> SideRecords;

    class OrderBook {
    private:
        SideRecords records;
        void update (Side side, u_int64_t qty, double price);

    public:
        OrderBook(){}
        OrderBook(SideRecords& records): records(records)  {}
        OrderBook(SideRecords&& records): records(records)  {}

        void add(OrderData newOrder);

        std::vector<std::pair<double,uint64_t>> getTop(Side side, size_t topN);

        SideRecords getSideRecords(); 
    };
}

#endif
