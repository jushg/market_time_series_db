#include <gtest/gtest.h>
#include <sstream>
#include "commands.hpp"
#include "shared.hpp"
#include "storages.hpp"
#include "TempDirFixture.hpp"
#include "OrderDataBuilder.hpp"

// Capture stdout from a command so we can inspect printed output
static std::string captureExecute(BaseCommand& cmd) {
    std::ostringstream buf;
    auto* old = std::cout.rdbuf(buf.rdbuf());
    cmd.execute();
    std::cout.rdbuf(old);
    return buf.str();
}

class InsertQueryTest : public TempDirFixture {
protected:
    std::shared_ptr<storage::TimeIndex> timeIdx;

    void SetUp() override {
        TempDirFixture::SetUp();
        timeIdx = std::make_shared<storage::TimeIndex>(symbol, rootDir);
    }

    CommonConfig makeConfig() {
        return CommonConfig{rootDir, symbol, timeIdx};
    }

    void insert(model::OrderData order) {
        InsertEntryCommand cmd(std::move(order), makeConfig());
        cmd.execute();
    }

    std::string query(uint64_t ts) {
        QueryConfig qcfg;
        QuerySingleCommand cmd(ts, makeConfig(), std::move(qcfg));
        return captureExecute(cmd);
    }
};

// ---------------------------------------------------------------------------
// Query before any data → empty result
// ---------------------------------------------------------------------------
TEST_F(InsertQueryTest, QueryBeforeAnyDataPrintsEmpty) {
    auto out = query(T1);
    EXPECT_NE(out.find("Empty"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Insert NEW → Query → book shows the order
// ---------------------------------------------------------------------------
TEST_F(InsertQueryTest, InsertNewThenQuery) {
    insert(makeBuyNew(symbol, T1, 1, 50.0, 100));
    auto out = query(T1);
    // 50.0 price and 100 qty should appear in the output
    EXPECT_NE(out.find("50"), std::string::npos);
    EXPECT_NE(out.find("100"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Insert NEW then TRADE → qty reduced
// ---------------------------------------------------------------------------
TEST_F(InsertQueryTest, InsertNewThenTrade) {
    insert(makeBuyNew(symbol,   T1,   1, 50.0, 100));
    insert(makeBuyTrade(symbol, T1+1, 2, 50.0, 30));
    auto out = query(T1 + 2);
    // Remaining qty = 70
    EXPECT_NE(out.find("70"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Insert NEW then CANCEL to zero → getTop returns nothing at that level
// ---------------------------------------------------------------------------
TEST_F(InsertQueryTest, InsertNewThenCancelToZero) {
    insert(makeBuyNew(symbol,    T1,   1, 50.0, 100));
    insert(makeBuyCancel(symbol, T1+1, 2, 50.0, 100));
    auto out = query(T1 + 2);
    // The 50.0 level should be gone (qty == 0 is skipped by getTop)
    // Result should show N.A for buy1 since book is empty at that level
    EXPECT_NE(out.find("N.A"), std::string::npos);
}

// ---------------------------------------------------------------------------
// TRADE updates lastTradePrice / lastTradeQty
// ---------------------------------------------------------------------------
TEST_F(InsertQueryTest, TradeUpdatesLastTrade) {
    insert(makeBuyNew(symbol,   T1,   1, 50.0, 100));
    insert(makeBuyTrade(symbol, T1+1, 2, 50.0, 25));
    auto out = query(T1 + 2);
    // last trade price = 50.0, qty = 25
    EXPECT_NE(out.find("50"), std::string::npos);
    EXPECT_NE(out.find("25"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Multiple price levels — top 5 ordering
// ---------------------------------------------------------------------------
TEST_F(InsertQueryTest, MultipleBuyLevels) {
    insert(makeBuyNew(symbol, T1,   1, 50.0, 100));
    insert(makeBuyNew(symbol, T1+1, 2, 51.0, 200));
    insert(makeBuyNew(symbol, T1+2, 3, 49.0, 300));
    auto out = query(T1 + 3);
    // The output iterates i=4..0, printing buy5→buy1.
    // With 3 levels: 49.0 is printed first (buy3), then 50.0 (buy2), then 51.0 (buy1).
    auto pos51 = out.find("51");
    auto pos50 = out.find("50");
    auto pos49 = out.find("49");
    ASSERT_NE(pos51, std::string::npos);
    ASSERT_NE(pos50, std::string::npos);
    ASSERT_NE(pos49, std::string::npos);
    // In the formatted output: buy5(N.A) buy4(N.A) buy3(49) buy2(50) buy1(51)
    EXPECT_LT(pos49, pos50);
    EXPECT_LT(pos50, pos51);
}

// ---------------------------------------------------------------------------
// Query at exact timestamp vs. earlier timestamp
// ---------------------------------------------------------------------------
TEST_F(InsertQueryTest, QueryAtExactTimestampSeesOrder) {
    insert(makeBuyNew(symbol, T1, 1, 50.0, 100));
    // Query at T1+1 uses the file containing T1 and applies orders up to T1+1
    auto out = query(T1 + 1);
    EXPECT_NE(out.find("50"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Index is not empty after insert
// ---------------------------------------------------------------------------
TEST_F(InsertQueryTest, IndexNotEmptyAfterInsert) {
    EXPECT_TRUE(timeIdx->isEmpty());
    insert(makeBuyNew(symbol, T1, 1, 50.0, 100));
    EXPECT_FALSE(timeIdx->isEmpty());
}

// ---------------------------------------------------------------------------
// Multiple inserts in same period go into same file
// ---------------------------------------------------------------------------
TEST_F(InsertQueryTest, TwoInsertsInSamePeriod) {
    insert(makeBuyNew(symbol, T1,   1, 50.0, 100));
    insert(makeBuyNew(symbol, T1+1, 2, 51.0, 200));
    auto out = query(T1 + 2);
    EXPECT_NE(out.find("50"), std::string::npos);
    EXPECT_NE(out.find("51"), std::string::npos);
}
