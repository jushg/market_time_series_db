#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "shared.hpp"
#include "storages.hpp"
#include "storage_models.hpp"
#include "models.hpp"
#include "TempDirFixture.hpp"
#include "OrderDataBuilder.hpp"

// ---------------------------------------------------------------------------
// isSamePeriod
// ---------------------------------------------------------------------------
TEST(SharedOpsTest, IsSamePeriodIdentical) {
    EXPECT_TRUE(isSamePeriod(T1, T1));
}

TEST(SharedOpsTest, IsSamePeriodExactlyPeriodApart) {
    // |T1 - (T1+PERIOD)| == PERIOD → same period (boundary is inclusive)
    EXPECT_TRUE(isSamePeriod(T1, T1 + PERIOD));
}

TEST(SharedOpsTest, IsSamePeriodOneNsOverBoundary) {
    EXPECT_FALSE(isSamePeriod(T1, T1 + PERIOD + 1));
}

TEST(SharedOpsTest, IsSamePeriodSymmetric) {
    EXPECT_EQ(isSamePeriod(T1, T2), isSamePeriod(T2, T1));
}

TEST(SharedOpsTest, IsSamePeriodDifferentPeriods) {
    // T2 is PERIOD+1 ahead of T1
    EXPECT_FALSE(isSamePeriod(T1, T2));
}

// ---------------------------------------------------------------------------
// swapIfIsTrade
// ---------------------------------------------------------------------------
TEST(SharedOpsTest, SwapIfIsTradeUpdatesOnTrade) {
    storage_model::LastTradeRecord lt;  // qty/price/ts all -1
    model::OrderData trade = makeBuyTrade("SYM", T1, 42, 99.5, 100);
    swapIfIsTrade(lt, trade);
    EXPECT_EQ(lt.qty, 100u);
    EXPECT_DOUBLE_EQ(lt.price, 99.5);
    EXPECT_EQ(lt.timestamp, T1);
}

TEST(SharedOpsTest, SwapIfIsTradeDoesNotUpdateOnNew) {
    storage_model::LastTradeRecord lt;
    model::OrderData newOrd = makeBuyNew("SYM", T1, 1, 50.0, 100);
    swapIfIsTrade(lt, newOrd);
    // Should not have changed — LastTradeRecord default is {-1,-1,-1}
    EXPECT_EQ(lt.qty, static_cast<uint64_t>(-1));
}

TEST(SharedOpsTest, SwapIfIsTradeDoesNotUpdateOnCancel) {
    storage_model::LastTradeRecord lt;
    model::OrderData cancel = makeBuyCancel("SYM", T1, 1, 50.0, 100);
    swapIfIsTrade(lt, cancel);
    EXPECT_EQ(lt.qty, static_cast<uint64_t>(-1));
}

TEST(SharedOpsTest, SwapIfIsTradeOverwritesPreviousTrade) {
    storage_model::LastTradeRecord lt;
    model::OrderData t1 = makeBuyTrade("SYM", T1,   1, 50.0, 100);
    model::OrderData t2 = makeSellTrade("SYM", T1+1, 2, 55.0, 50);
    swapIfIsTrade(lt, t1);
    swapIfIsTrade(lt, t2);
    EXPECT_DOUBLE_EQ(lt.price, 55.0);
    EXPECT_EQ(lt.qty, 50u);
}

// ---------------------------------------------------------------------------
// getOrderBookSnapshot (after ops.cpp .dat fix)
// ---------------------------------------------------------------------------
class SharedOpsSnapshotTest : public TempDirFixture {
protected:
    std::shared_ptr<storage::TimeIndex> timeIdx;

    void SetUp() override {
        TempDirFixture::SetUp();
        timeIdx = std::make_shared<storage::TimeIndex>(symbol, rootDir);
    }

    // Write a data file with the given book/orders/lastTrade at timestamp ts
    void writeDataFile(uint64_t ts, model::OrderBook& book,
                       std::vector<model::OrderData>& orders,
                       storage_model::LastTradeRecord& lt) {
        auto fname = storage::getFileName(rootDir, symbol, ts);
        std::ofstream h(fname, std::ios::out | std::ios::binary);
        storage::write(h, book, orders, lt);
        h.close();
        timeIdx->loadIdxFromFile();
    }
};

TEST_F(SharedOpsSnapshotTest, ReturnsEmptyBookWhenIndexEmpty) {
    CommonConfig cfg{rootDir, symbol, timeIdx};
    auto book = getOrderBookSnapshot(cfg, T1);
    EXPECT_TRUE(book.getTop(model::Side::BUY, 5).empty());
    EXPECT_TRUE(book.getTop(model::Side::SELL, 5).empty());
}

TEST_F(SharedOpsSnapshotTest, ReturnsEmptyBookWhenTimestampBeforeAllData) {
    model::OrderBook b;
    std::vector<model::OrderData> orders;
    storage_model::LastTradeRecord lt;
    writeDataFile(T2, b, orders, lt);

    CommonConfig cfg{rootDir, symbol, timeIdx};
    // Query before T2: findNearestIndexPrior returns -1 → empty book
    auto book = getOrderBookSnapshot(cfg, T1);
    EXPECT_TRUE(book.getTop(model::Side::BUY, 5).empty());
}

TEST_F(SharedOpsSnapshotTest, AppliesOrdersUpToTimestamp) {
    // File at T1: empty base state, two orders: NEW at T1 and NEW at T1+5
    model::OrderBook base;
    std::vector<model::OrderData> orders = {
        makeBuyNew(symbol, T1,   1, 50.0, 100),
        makeBuyNew(symbol, T1+5, 2, 51.0, 200),
    };
    storage_model::LastTradeRecord lt;
    writeDataFile(T1, base, orders, lt);

    CommonConfig cfg{rootDir, symbol, timeIdx};
    // Snapshot at T1: applies the order at T1, stops before T1+5
    auto book = getOrderBookSnapshot(cfg, T1);
    auto top = book.getTop(model::Side::BUY, 5);
    ASSERT_EQ(top.size(), 1u);
    EXPECT_DOUBLE_EQ(top[0].first, 50.0);
    EXPECT_EQ(top[0].second, 100u);
}

TEST_F(SharedOpsSnapshotTest, AppliesAllOrdersWhenTimestampCoversAll) {
    model::OrderBook base;
    std::vector<model::OrderData> orders = {
        makeBuyNew(symbol, T1,   1, 50.0, 100),
        makeBuyNew(symbol, T1+5, 2, 51.0, 200),
    };
    storage_model::LastTradeRecord lt;
    writeDataFile(T1, base, orders, lt);

    CommonConfig cfg{rootDir, symbol, timeIdx};
    auto book = getOrderBookSnapshot(cfg, T1 + 100);
    auto top = book.getTop(model::Side::BUY, 5);
    ASSERT_EQ(top.size(), 2u);
    EXPECT_DOUBLE_EQ(top[0].first, 51.0);
    EXPECT_DOUBLE_EQ(top[1].first, 50.0);
}
