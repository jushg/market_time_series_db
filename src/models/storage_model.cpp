#include <stdint.h>

namespace storage_model {
    struct Metadata {
        size_t buyCnt;
        size_t sellCnt;
        size_t orderCnt;
    };

    struct LastTradeRecord {
        uint64_t qty;
        double price;
        uint64_t timestamp;
    };

    struct BaseStateRecord {
        uint64_t qty;
        double price;
    };

    struct OrderRecord {
        char category;
        char side;
        uint64_t timestamp;
        uint64_t qty;
        double price;
    };
}