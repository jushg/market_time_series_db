#pragma once
#include "models.hpp"
#include "shared.hpp"   // for PERIOD

// Convenient timestamp constants (nanoseconds)
static constexpr uint64_t T1 = 1'000'000'000'000ULL;            // period 1 start
static constexpr uint64_t T2 = T1 + PERIOD + 1;                 // period 2 start
static constexpr uint64_t T3 = T2 + PERIOD + 1;                 // period 3 start

// Builder helpers — reduce boilerplate in test bodies.
inline model::OrderData makeBuyNew(const std::string& sym, uint64_t ts, uint64_t id,
                                    double price, uint32_t qty) {
    return model::OrderData(sym, ts, id, model::Side::BUY, model::Category::NEW, qty, price);
}

inline model::OrderData makeSellNew(const std::string& sym, uint64_t ts, uint64_t id,
                                     double price, uint32_t qty) {
    return model::OrderData(sym, ts, id, model::Side::SELL, model::Category::NEW, qty, price);
}

inline model::OrderData makeBuyTrade(const std::string& sym, uint64_t ts, uint64_t id,
                                      double price, uint32_t qty) {
    return model::OrderData(sym, ts, id, model::Side::BUY, model::Category::TRADE, qty, price);
}

inline model::OrderData makeSellTrade(const std::string& sym, uint64_t ts, uint64_t id,
                                       double price, uint32_t qty) {
    return model::OrderData(sym, ts, id, model::Side::SELL, model::Category::TRADE, qty, price);
}

inline model::OrderData makeBuyCancel(const std::string& sym, uint64_t ts, uint64_t id,
                                       double price, uint32_t qty) {
    return model::OrderData(sym, ts, id, model::Side::BUY, model::Category::CANCEL, qty, price);
}
