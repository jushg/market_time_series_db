#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "shared.hpp"
#include "storages.hpp"
#include "commands.hpp"
#include "TempDirFixture.hpp"
#include "OrderDataBuilder.hpp"

class MergeStateTest : public TempDirFixture {
protected:
    std::shared_ptr<storage::TimeIndex> timeIdx;

    void SetUp() override {
        TempDirFixture::SetUp();
        timeIdx = std::make_shared<storage::TimeIndex>(symbol, rootDir);
    }

    CommonConfig makeConfig() { return CommonConfig{rootDir, symbol, timeIdx}; }

    // Write a raw period file and update the index.
    void seedFile(uint64_t ts, model::OrderBook& book,
                  std::vector<model::OrderData>& orders,
                  storage_model::LastTradeRecord& lt) {
        auto fname = storage::getFileName(rootDir, symbol, ts);
        std::ofstream h(fname, std::ios::out | std::ios::binary);
        storage::write(h, book, orders, lt);
        h.close();
        timeIdx->loadIdxFromFile();
    }

    void insert(model::OrderData order) {
        InsertEntryCommand cmd(std::move(order), makeConfig());
        cmd.execute();
    }
};

// ---------------------------------------------------------------------------
// mergeStateAndWrite: inserting into existing period merges correctly
// ---------------------------------------------------------------------------
TEST_F(MergeStateTest, MergeIntoExistingPeriod) {
    // Seed period with one order
    model::OrderBook base;
    std::vector<model::OrderData> orders1 = {makeBuyNew(symbol, T1, 1, 50.0, 100)};
    storage_model::LastTradeRecord lt;
    seedFile(T1, base, orders1, lt);

    // Merge a second order into the same period
    std::vector<model::OrderData> toMerge = {makeBuyNew(symbol, T1 + 1, 2, 51.0, 200)};
    auto cfg = makeConfig();
    mergeStateAndWrite(cfg, T1, T1 + 1, toMerge);

    // Read back the merged file
    auto nearestTs = timeIdx->findNearestIndexPrior(T1);
    ASSERT_NE(nearestTs, static_cast<uint64_t>(-1));
    storage::Reader reader;
    reader.loadData(storage::getFileName(rootDir, symbol, nearestTs));
    auto loadedOrders = reader.getOrders(symbol);

    // Both orders should be present in chronological order
    ASSERT_EQ(loadedOrders.size(), 2u);
    EXPECT_EQ(loadedOrders[0].id, 1u);   // original first
    EXPECT_EQ(loadedOrders[1].id, 2u);   // merged second
}

// ---------------------------------------------------------------------------
// mergeStateAndWrite: earlier-timestamp order goes before existing orders
// ---------------------------------------------------------------------------
TEST_F(MergeStateTest, MergeEarlierOrderIsPlacedFirst) {
    model::OrderBook base;
    std::vector<model::OrderData> orders1 = {makeBuyNew(symbol, T1 + 10, 2, 50.0, 100)};
    storage_model::LastTradeRecord lt;
    seedFile(T1, base, orders1, lt);

    // Merge an order with EARLIER timestamp
    std::vector<model::OrderData> toMerge = {makeBuyNew(symbol, T1 + 1, 1, 51.0, 200)};
    auto cfg = makeConfig();
    mergeStateAndWrite(cfg, T1, T1 + 10, toMerge);

    // mergeStateAndWrite names the new file after currentOrders.front().timestamp.
    // Since the merged order (T1+1) is earlier than the loaded order (T1+10),
    // the resulting file is named T1+1.dat, not T1.dat.
    auto nearestTs = timeIdx->findNearestIndexPrior(T1 + 2);
    ASSERT_NE(nearestTs, static_cast<uint64_t>(-1));
    storage::Reader reader;
    reader.loadData(storage::getFileName(rootDir, symbol, nearestTs));
    auto loaded = reader.getOrders(symbol);

    ASSERT_EQ(loaded.size(), 2u);
    EXPECT_EQ(loaded[0].id, 1u);  // earlier ts=T1+1 comes first
    EXPECT_EQ(loaded[1].id, 2u);  // later ts=T1+10 comes second
}

// ---------------------------------------------------------------------------
// mergeStateAndWrite: TRADE in merged orders updates lastTrade in the file
// ---------------------------------------------------------------------------
TEST_F(MergeStateTest, MergeTradeUpdatesLastTrade) {
    model::OrderBook base;
    std::vector<model::OrderData> seedOrders = {makeBuyNew(symbol, T1, 1, 50.0, 100)};
    storage_model::LastTradeRecord lt;
    seedFile(T1, base, seedOrders, lt);

    std::vector<model::OrderData> toMerge = {makeBuyTrade(symbol, T1 + 1, 2, 50.0, 30)};
    auto cfg = makeConfig();
    mergeStateAndWrite(cfg, T1, T1 + 1, toMerge);

    auto nearestTs = timeIdx->findNearestIndexPrior(T1);
    storage::Reader reader;
    reader.loadData(storage::getFileName(rootDir, symbol, nearestTs));
    auto loadedLt = reader.getLastTrade();

    EXPECT_EQ(loadedLt.qty, 30u);
    EXPECT_DOUBLE_EQ(loadedLt.price, 50.0);
}

// ---------------------------------------------------------------------------
// InsertEntryCommand into an already-populated period merges correctly
// (InsertEntryCommand uses mergeStateAndWrite internally)
// ---------------------------------------------------------------------------
TEST_F(MergeStateTest, InsertCommandMergesIntoExistingPeriod) {
    // First insert creates the period file
    insert(makeBuyNew(symbol, T1, 1, 50.0, 100));
    // Second insert in same period must merge
    insert(makeBuyNew(symbol, T1 + 1, 2, 51.0, 200));

    // Now query via reader to verify both orders are stored
    auto nearestTs = timeIdx->findNearestIndexPrior(T1 + 1);
    ASSERT_NE(nearestTs, static_cast<uint64_t>(-1));
    storage::Reader reader;
    reader.loadData(storage::getFileName(rootDir, symbol, nearestTs));
    auto loaded = reader.getOrders(symbol);

    ASSERT_EQ(loaded.size(), 2u);
}
