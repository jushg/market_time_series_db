

#include <unordered_map>
#include <map>
#include <stdint.h>
#include <queue>
#include <vector>
#include <algorithm>
#include <utility>
#include "./order.cpp"

namespace order_book {
    typedef std::map<double, uint64_t> OrderRecord;

    typedef std::unordered_map<order::Side, OrderRecord> SideRecords;

    class OrderBook {
    private:
        SideRecords records;
        void update (order::Side side, u_int64_t qty, double price) {
            records[side][price] += qty;

            // DEBUG mode
            assert(records[side][price] >= 0);
        }

    public:
        OrderBook(){}
        OrderBook(SideRecords& records): records(records)  {}

        void add(const order::OrderData& newOrder) {
            switch (newOrder.category)
            {
            case order::Category::NEW:
                update(newOrder.side,newOrder.qty, newOrder.price);
                break;
            case order::Category::TRADE:
                update(newOrder.side,(-1) * newOrder.qty, newOrder.price);
                break;

            case order::Category::CANCEL:
                update(newOrder.side,(-1) * newOrder.qty, newOrder.price);
                break;

            default:
                // Add exception
                break;
            }
        };

        std::vector<std::pair<double,uint64_t>> getTop(order::Side side, size_t topN) {
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

        SideRecords getSideRecords() {
            return records;
        }

    };
}

