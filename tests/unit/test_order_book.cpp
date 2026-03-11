#include <gtest/gtest.h>
#include "models.hpp"

using namespace model;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static OrderData newOrder(Side side, double price, uint32_t qty, uint64_t ts = 1) {
    return OrderData("SYM", ts, 1, side, Category::NEW, qty, price);
}
static OrderData tradeOrder(Side side, double price, uint32_t qty, uint64_t ts = 2) {
    return OrderData("SYM", ts, 2, side, Category::TRADE, qty, price);
}
static OrderData cancelOrder(Side side, double price, uint32_t qty, uint64_t ts = 3) {
    return OrderData("SYM", ts, 3, side, Category::CANCEL, qty, price);
}

// ---------------------------------------------------------------------------
// NEW increases quantity
// ---------------------------------------------------------------------------
TEST(OrderBookTest, NewBuyIncreasesQty) {
    OrderBook book;
    book.add(newOrder(Side::BUY, 50.0, 100));
    auto top = book.getTop(Side::BUY, 5);
    ASSERT_EQ(top.size(), 1u);
    EXPECT_DOUBLE_EQ(top[0].first, 50.0);
    EXPECT_EQ(top[0].second, 100u);
}

TEST(OrderBookTest, NewSellIncreasesQty) {
    OrderBook book;
    book.add(newOrder(Side::SELL, 55.0, 200));
    auto top = book.getTop(Side::SELL, 5);
    ASSERT_EQ(top.size(), 1u);
    EXPECT_DOUBLE_EQ(top[0].first, 55.0);
    EXPECT_EQ(top[0].second, 200u);
}

TEST(OrderBookTest, MultipleNewsSamePrice) {
    OrderBook book;
    book.add(newOrder(Side::BUY, 50.0, 100));
    book.add(newOrder(Side::BUY, 50.0, 50));
    auto top = book.getTop(Side::BUY, 5);
    ASSERT_EQ(top.size(), 1u);
    EXPECT_EQ(top[0].second, 150u);
}

// ---------------------------------------------------------------------------
// TRADE decreases quantity
// ---------------------------------------------------------------------------
TEST(OrderBookTest, TradeReducesQty) {
    OrderBook book;
    book.add(newOrder(Side::BUY, 50.0, 100));
    book.add(tradeOrder(Side::BUY, 50.0, 30));
    auto top = book.getTop(Side::BUY, 5);
    ASSERT_EQ(top.size(), 1u);
    EXPECT_EQ(top[0].second, 70u);
}

TEST(OrderBookTest, TradeToZeroQty) {
    OrderBook book;
    book.add(newOrder(Side::BUY, 50.0, 100));
    book.add(tradeOrder(Side::BUY, 50.0, 100));
    auto top = book.getTop(Side::BUY, 5);
    // qty == 0 → getTop skips it
    EXPECT_TRUE(top.empty());
}

// ---------------------------------------------------------------------------
// CANCEL decreases quantity
// ---------------------------------------------------------------------------
TEST(OrderBookTest, CancelReducesQty) {
    OrderBook book;
    book.add(newOrder(Side::BUY, 50.0, 100));
    book.add(cancelOrder(Side::BUY, 50.0, 40));
    auto top = book.getTop(Side::BUY, 5);
    ASSERT_EQ(top.size(), 1u);
    EXPECT_EQ(top[0].second, 60u);
}

// ---------------------------------------------------------------------------
// getTop ordering: BUY → highest price first; SELL → lowest price first
// ---------------------------------------------------------------------------
TEST(OrderBookTest, GetTopBuyHighestFirst) {
    OrderBook book;
    book.add(newOrder(Side::BUY, 50.0, 10));
    book.add(newOrder(Side::BUY, 51.0, 20));
    book.add(newOrder(Side::BUY, 49.0, 30));
    auto top = book.getTop(Side::BUY, 5);
    ASSERT_EQ(top.size(), 3u);
    EXPECT_DOUBLE_EQ(top[0].first, 51.0);
    EXPECT_DOUBLE_EQ(top[1].first, 50.0);
    EXPECT_DOUBLE_EQ(top[2].first, 49.0);
}

TEST(OrderBookTest, GetTopSellLowestFirst) {
    OrderBook book;
    book.add(newOrder(Side::SELL, 55.0, 10));
    book.add(newOrder(Side::SELL, 53.0, 20));
    book.add(newOrder(Side::SELL, 57.0, 30));
    auto top = book.getTop(Side::SELL, 5);
    ASSERT_EQ(top.size(), 3u);
    EXPECT_DOUBLE_EQ(top[0].first, 53.0);
    EXPECT_DOUBLE_EQ(top[1].first, 55.0);
    EXPECT_DOUBLE_EQ(top[2].first, 57.0);
}

// ---------------------------------------------------------------------------
// getTop skips zero-qty entries
// ---------------------------------------------------------------------------
TEST(OrderBookTest, GetTopSkipsZeroQtyEntries) {
    OrderBook book;
    book.add(newOrder(Side::BUY, 50.0, 100));
    book.add(newOrder(Side::BUY, 51.0, 50));
    // Cancel all of 51.0
    book.add(cancelOrder(Side::BUY, 51.0, 50));
    auto top = book.getTop(Side::BUY, 5);
    ASSERT_EQ(top.size(), 1u);
    EXPECT_DOUBLE_EQ(top[0].first, 50.0);
}

// ---------------------------------------------------------------------------
// getTop returns fewer than topN if not enough levels
// ---------------------------------------------------------------------------
TEST(OrderBookTest, GetTopFewerThanN) {
    OrderBook book;
    book.add(newOrder(Side::BUY, 50.0, 100));
    book.add(newOrder(Side::BUY, 51.0, 200));
    auto top = book.getTop(Side::BUY, 5);
    EXPECT_EQ(top.size(), 2u);
}

TEST(OrderBookTest, GetTopEmptyBook) {
    OrderBook book;
    EXPECT_TRUE(book.getTop(Side::BUY, 5).empty());
    EXPECT_TRUE(book.getTop(Side::SELL, 5).empty());
}

// ---------------------------------------------------------------------------
// BUY and SELL sides are independent
// ---------------------------------------------------------------------------
TEST(OrderBookTest, BuyAndSellSidesIndependent) {
    OrderBook book;
    book.add(newOrder(Side::BUY, 50.0, 100));
    book.add(newOrder(Side::SELL, 55.0, 200));
    EXPECT_EQ(book.getTop(Side::BUY, 5).size(), 1u);
    EXPECT_EQ(book.getTop(Side::SELL, 5).size(), 1u);
    EXPECT_DOUBLE_EQ(book.getTop(Side::BUY, 5)[0].first, 50.0);
    EXPECT_DOUBLE_EQ(book.getTop(Side::SELL, 5)[0].first, 55.0);
}

// ---------------------------------------------------------------------------
// add(vector) overload
// ---------------------------------------------------------------------------
TEST(OrderBookTest, AddVectorOfOrders) {
    OrderBook book;
    std::vector<OrderData> orders = {
        newOrder(Side::BUY, 50.0, 100),
        newOrder(Side::BUY, 51.0, 200),
        tradeOrder(Side::BUY, 50.0, 30),
    };
    book.add(orders);
    auto top = book.getTop(Side::BUY, 5);
    ASSERT_EQ(top.size(), 2u);
    EXPECT_DOUBLE_EQ(top[0].first, 51.0);
    EXPECT_EQ(top[0].second, 200u);
    EXPECT_DOUBLE_EQ(top[1].first, 50.0);
    EXPECT_EQ(top[1].second, 70u);
}

// ---------------------------------------------------------------------------
// Bug regression: TRADE on non-existent price causes unsigned wrap-around.
// The qty stored becomes a huge value rather than an error being raised.
// This test documents the current (incorrect) behavior.
// ---------------------------------------------------------------------------
TEST(OrderBookTest, DISABLED_TradeNonExistentPriceUnderflowsDueToUnsignedWrap) {
    OrderBook book;
    // TRADE on a price level that was never seeded with a NEW order
    book.add(tradeOrder(Side::BUY, 99.0, 10));
    auto top = book.getTop(Side::BUY, 5);
    // After fix this should be EXPECT_TRUE(top.empty())
    // Currently the qty wraps to a huge positive value, so there IS an entry
    ASSERT_EQ(top.size(), 1u);
    EXPECT_GT(top[0].second, 0u);  // corrupted: huge qty, not 0 or negative
}

// ---------------------------------------------------------------------------
// getSideRecords returns the underlying map
// ---------------------------------------------------------------------------
TEST(OrderBookTest, GetSideRecordsReflectsState) {
    OrderBook book;
    book.add(newOrder(Side::BUY, 50.0, 100));
    auto records = book.getSideRecords();
    ASSERT_EQ(records[Side::BUY].count(50.0), 1u);
    EXPECT_EQ(records[Side::BUY].at(50.0), 100u);
}

// ---------------------------------------------------------------------------
// Construct from existing SideRecords
// ---------------------------------------------------------------------------
TEST(OrderBookTest, ConstructFromSideRecords) {
    SideRecords sr;
    sr[Side::BUY][50.0] = 100;
    sr[Side::SELL][55.0] = 200;
    OrderBook book(sr);
    ASSERT_EQ(book.getTop(Side::BUY, 5).size(), 1u);
    EXPECT_EQ(book.getTop(Side::BUY, 5)[0].second, 100u);
    ASSERT_EQ(book.getTop(Side::SELL, 5).size(), 1u);
    EXPECT_EQ(book.getTop(Side::SELL, 5)[0].second, 200u);
}
