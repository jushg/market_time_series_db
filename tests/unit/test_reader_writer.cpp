#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "storages.hpp"
#include "models.hpp"
#include "storage_models.hpp"
#include "TempDirFixture.hpp"
#include "OrderDataBuilder.hpp"

class ReaderWriterTest : public TempDirFixture {
protected:
    std::string filePath;

    void SetUp() override {
        TempDirFixture::SetUp();
        storage::createSymbolDirectoryIfNotExist(rootDir, symbol);
        filePath = storage::getFileName(rootDir, symbol, T1);
    }

    // Write book + orders + lastTrade to filePath, return it.
    void writeFile(model::OrderBook& book,
                   std::vector<model::OrderData>& orders,
                   storage_model::LastTradeRecord& lt) {
        std::ofstream h(filePath, std::ios::out | std::ios::binary);
        storage::write(h, book, orders, lt);
        h.close();
    }
};

// ---------------------------------------------------------------------------
// Empty book roundtrip
// ---------------------------------------------------------------------------
TEST_F(ReaderWriterTest, EmptyBookRoundtrip) {
    model::OrderBook book;
    std::vector<model::OrderData> orders;
    storage_model::LastTradeRecord lt;

    writeFile(book, orders, lt);

    storage::Reader reader;
    reader.loadData(filePath);
    EXPECT_TRUE(reader.buyRecords.empty());
    EXPECT_TRUE(reader.sellRecords.empty());
    EXPECT_TRUE(reader.orders.empty());
}

// ---------------------------------------------------------------------------
// Single order roundtrip
// ---------------------------------------------------------------------------
TEST_F(ReaderWriterTest, SingleOrderRoundtrip) {
    model::OrderBook book;
    std::vector<model::OrderData> orders = {makeBuyNew(symbol, T1, 1, 50.0, 100)};
    storage_model::LastTradeRecord lt;

    writeFile(book, orders, lt);

    storage::Reader reader;
    reader.loadData(filePath);
    ASSERT_EQ(reader.orders.size(), 1u);
    auto loaded = reader.getOrders(symbol);
    ASSERT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded[0].timestamp, T1);
    EXPECT_EQ(loaded[0].id, 1u);
    EXPECT_EQ(loaded[0].side, model::Side::BUY);
    EXPECT_EQ(loaded[0].category, model::Category::NEW);
    EXPECT_EQ(loaded[0].qty, 100u);
    EXPECT_DOUBLE_EQ(loaded[0].price, 50.0);
}

// ---------------------------------------------------------------------------
// Base state (SideRecords) roundtrip
// ---------------------------------------------------------------------------
TEST_F(ReaderWriterTest, BaseStateRoundtrip) {
    model::SideRecords sr;
    sr[model::Side::BUY][50.0] = 100;
    sr[model::Side::BUY][51.0] = 200;
    sr[model::Side::SELL][55.0] = 300;
    model::OrderBook book(sr);
    std::vector<model::OrderData> orders;
    storage_model::LastTradeRecord lt;

    writeFile(book, orders, lt);

    storage::Reader reader;
    reader.loadData(filePath);
    EXPECT_EQ(reader.buyRecords.size(), 2u);
    EXPECT_EQ(reader.sellRecords.size(), 1u);

    auto loaded = reader.getRecords();
    EXPECT_EQ(loaded[model::Side::BUY][50.0], 100u);
    EXPECT_EQ(loaded[model::Side::BUY][51.0], 200u);
    EXPECT_EQ(loaded[model::Side::SELL][55.0], 300u);
}

// ---------------------------------------------------------------------------
// LastTradeRecord roundtrip
// ---------------------------------------------------------------------------
TEST_F(ReaderWriterTest, LastTradeRoundtrip) {
    model::OrderBook book;
    std::vector<model::OrderData> orders;
    storage_model::LastTradeRecord lt;
    lt.qty = 42;
    lt.price = 99.5;
    lt.timestamp = T1;

    writeFile(book, orders, lt);

    storage::Reader reader;
    reader.loadData(filePath);
    auto loaded = reader.getLastTrade();
    EXPECT_EQ(loaded.qty, 42u);
    EXPECT_DOUBLE_EQ(loaded.price, 99.5);
    EXPECT_EQ(loaded.timestamp, T1);
}

// ---------------------------------------------------------------------------
// Mixed side orders roundtrip
// ---------------------------------------------------------------------------
TEST_F(ReaderWriterTest, MixedOrdersRoundtrip) {
    model::OrderBook book;
    std::vector<model::OrderData> orders = {
        makeBuyNew(symbol,  T1,     1, 50.0, 100),
        makeSellNew(symbol, T1+1,   2, 55.0, 200),
        makeBuyTrade(symbol,T1+2,   3, 50.0, 30),
        makeBuyCancel(symbol,T1+3,  4, 50.0, 10),
    };
    storage_model::LastTradeRecord lt;

    writeFile(book, orders, lt);

    storage::Reader reader;
    reader.loadData(filePath);
    ASSERT_EQ(reader.orders.size(), 4u);

    auto loaded = reader.getOrders(symbol);
    ASSERT_EQ(loaded.size(), 4u);

    EXPECT_EQ(loaded[0].side, model::Side::BUY);
    EXPECT_EQ(loaded[0].category, model::Category::NEW);
    EXPECT_EQ(loaded[1].side, model::Side::SELL);
    EXPECT_EQ(loaded[1].category, model::Category::NEW);
    EXPECT_EQ(loaded[2].category, model::Category::TRADE);
    EXPECT_EQ(loaded[3].category, model::Category::CANCEL);
}

// ---------------------------------------------------------------------------
// 50-order roundtrip
// ---------------------------------------------------------------------------
TEST_F(ReaderWriterTest, FiftyOrdersRoundtrip) {
    model::OrderBook book;
    std::vector<model::OrderData> orders;
    for (int i = 0; i < 50; i++) {
        orders.push_back(makeBuyNew(symbol, T1 + i, i + 1, 50.0 + i, 10 * (i + 1)));
    }
    storage_model::LastTradeRecord lt;

    writeFile(book, orders, lt);

    storage::Reader reader;
    reader.loadData(filePath);
    EXPECT_EQ(reader.orders.size(), 50u);
    auto loaded = reader.getOrders(symbol);
    ASSERT_EQ(loaded.size(), 50u);
    for (int i = 0; i < 50; i++) {
        EXPECT_EQ(loaded[i].id, static_cast<uint64_t>(i + 1));
        EXPECT_DOUBLE_EQ(loaded[i].price, 50.0 + i);
        EXPECT_EQ(loaded[i].qty, static_cast<uint64_t>(10 * (i + 1)));
    }
}

// ---------------------------------------------------------------------------
// Reading non-existent file is safe (no crash)
// ---------------------------------------------------------------------------
TEST_F(ReaderWriterTest, LoadNonExistentFileDoesNotCrash) {
    storage::Reader reader;
    EXPECT_NO_THROW(reader.loadData("/tmp/nonexistent_mkt_test_file.dat"));
    EXPECT_TRUE(reader.orders.empty());
    EXPECT_TRUE(reader.buyRecords.empty());
    EXPECT_TRUE(reader.sellRecords.empty());
}

// ---------------------------------------------------------------------------
// deleteDataAt removes the file
// ---------------------------------------------------------------------------
TEST_F(ReaderWriterTest, DeleteDataAtRemovesFile) {
    model::OrderBook book;
    std::vector<model::OrderData> orders;
    storage_model::LastTradeRecord lt;
    writeFile(book, orders, lt);
    ASSERT_TRUE(std::filesystem::exists(filePath));

    storage::deleteDataAt(filePath);
    EXPECT_FALSE(std::filesystem::exists(filePath));
}
