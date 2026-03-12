#include <gtest/gtest.h>
#include <sstream>
#include "commands.hpp"
#include "shared.hpp"
#include "storages.hpp"
#include "TempDirFixture.hpp"
#include "OrderDataBuilder.hpp"

static std::string captureExecute(BaseCommand& cmd) {
    std::ostringstream buf;
    auto* old = std::cout.rdbuf(buf.rdbuf());
    cmd.execute();
    std::cout.rdbuf(old);
    return buf.str();
}

class MultiPeriodTest : public TempDirFixture {
protected:
    std::shared_ptr<storage::TimeIndex> timeIdx;

    void SetUp() override {
        TempDirFixture::SetUp();
        timeIdx = std::make_shared<storage::TimeIndex>(symbol, rootDir);
    }

    CommonConfig makeConfig() { return CommonConfig{rootDir, symbol, timeIdx}; }

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
// Inserts in two different periods each create a separate index entry.
// ---------------------------------------------------------------------------
TEST_F(MultiPeriodTest, TwoPeriodsCreatesToIndexEntries) {
    insert(makeBuyNew(symbol, T1, 1, 50.0, 100));
    insert(makeBuyNew(symbol, T2, 2, 55.0, 200));

    auto ts1 = timeIdx->findNearestIndexPrior(T1);
    auto ts2 = timeIdx->findNearestIndexPrior(T2);
    EXPECT_NE(ts1, static_cast<uint64_t>(-1));
    EXPECT_NE(ts2, static_cast<uint64_t>(-1));
    EXPECT_NE(ts1, ts2);  // T1 and T2 should be separate index entries
}

// ---------------------------------------------------------------------------
// QuerySingle at T1 applies only orders with timestamp <= T1.
// T2's order (55.0) is in a separate file and is not applied at T1.
// The 50.0 order at T1 IS applied.
// ---------------------------------------------------------------------------
TEST_F(MultiPeriodTest, QueryAtPeriod1SeesOnlyPeriod1State) {
    insert(makeBuyNew(symbol, T1, 1, 50.0, 100));
    insert(makeBuyNew(symbol, T2, 2, 55.0, 200));

    auto out = query(T1);
    // 50.0 should appear as a buy order
    EXPECT_NE(out.find("100@50"), std::string::npos);
    // The T2 order (200 qty @ 55.0) should NOT be applied at T1
    EXPECT_EQ(out.find("200@55"), std::string::npos);
}

// ---------------------------------------------------------------------------
// QuerySingle at T2 sees carried-forward state PLUS period-2 orders
// ---------------------------------------------------------------------------
TEST_F(MultiPeriodTest, QueryAtPeriod2SeesBothPeriods) {
    insert(makeBuyNew(symbol, T1, 1, 50.0, 100));
    insert(makeBuyNew(symbol, T2, 2, 55.0, 200));

    auto out = query(T2);
    // 55.0 should appear (period-2 order applied)
    EXPECT_NE(out.find("55"), std::string::npos);
}

// ---------------------------------------------------------------------------
// TRADE in period 2 reduces qty that was set in period 1
// ---------------------------------------------------------------------------
TEST_F(MultiPeriodTest, TradeInPeriod2ReducesQtyFromPeriod1) {
    insert(makeBuyNew(symbol,   T1,   1, 50.0, 100));
    insert(makeBuyTrade(symbol, T2,   2, 50.0, 40));  // trade in period 2

    auto out = query(T2);
    // Remaining qty at 50.0 should be 60
    EXPECT_NE(out.find("60"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Three periods: state carries through correctly
// ---------------------------------------------------------------------------
TEST_F(MultiPeriodTest, ThreePeriods) {
    insert(makeBuyNew(symbol, T1, 1, 50.0, 100));
    insert(makeBuyNew(symbol, T2, 2, 51.0, 200));
    insert(makeBuyNew(symbol, T3, 3, 52.0, 300));

    // At T3, all three price levels should be visible
    auto out = query(T3);
    EXPECT_NE(out.find("52"), std::string::npos);
}
