#ifndef STORAGE_MODEL_HPP
#define STORAGE_MODEL_HPP

#include <stdint.h>
#include <stdlib.h>

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
        uint64_t timestamp;
        uint64_t id;
        char category;
        char side;
        uint64_t qty;
        double price;
        OrderRecord(){}
        OrderRecord(
            uint64_t timestamp,
            uint64_t id,
            char side,
            char category,
            uint32_t qty,
            double price)
            : timestamp(timestamp), id(id), side(side), category(category), qty(qty), price(price) {}
    };
}
#endif
