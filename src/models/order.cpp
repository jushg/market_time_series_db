
#include <string>

namespace order {
    enum Side {
        SELL,
        BUY
    };
    
    static const Side AllSide[] = { Side::BUY, Side::SELL };

    enum Category {
        TRADE,
        NEW,
        CANCEL
    };

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


}

