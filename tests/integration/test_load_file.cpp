#include <gtest/gtest.h>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "commands.hpp"
#include "shared.hpp"
#include "storages.hpp"
#include "TempDirFixture.hpp"
#include "OrderDataBuilder.hpp"

// LoadFileCommand input format: "<ts> <id> <symbol> <side> <category> <price> <qty>"
// (see LoadFileCommand::parseInputToOrderData)

static std::string makeOrderLine(uint64_t ts, uint64_t id, const std::string& sym,
                                  const std::string& side, const std::string& cat,
                                  double price, uint64_t qty) {
    return std::to_string(ts) + " " + std::to_string(id) + " " + sym + " " +
           side + " " + cat + " " + std::to_string(price) + " " + std::to_string(qty);
}

class LoadFileTest : public TempDirFixture {
protected:
    std::shared_ptr<storage::TimeIndex> timeIdx;
    std::string testFile;

    void SetUp() override {
        TempDirFixture::SetUp();
        timeIdx = std::make_shared<storage::TimeIndex>(symbol, rootDir);
        testFile = (tempDir / "input.txt").string();
    }

    void writeInputFile(const std::vector<std::string>& lines) {
        std::ofstream f(testFile);
        for (const auto& l : lines) f << l << "\n";
    }

    CommonConfig makeConfig() { return CommonConfig{rootDir, symbol, timeIdx}; }

    void runLoad() {
        LoadFileCommand cmd(testFile, makeConfig());
        cmd.execute();
    }

    std::string captureQuery(uint64_t ts) {
        QueryConfig qcfg;
        QuerySingleCommand cmd(ts, makeConfig(), std::move(qcfg));
        std::ostringstream buf;
        auto* old = std::cout.rdbuf(buf.rdbuf());
        cmd.execute();
        std::cout.rdbuf(old);
        return buf.str();
    }
};

// ---------------------------------------------------------------------------
// Single-period file: data is written after flush at end of loop
// ---------------------------------------------------------------------------
TEST_F(LoadFileTest, SinglePeriodFileWritesData) {
    writeInputFile({
        makeOrderLine(T1,   1, symbol, "BUY", "NEW", 50.0, 100),
        makeOrderLine(T1+1, 2, symbol, "BUY", "NEW", 51.0, 200),
    });
    runLoad();
    EXPECT_FALSE(timeIdx->isEmpty());
    auto out = captureQuery(T1 + 1);
    EXPECT_NE(out.find("50"), std::string::npos);
    EXPECT_NE(out.find("51"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Two-period file: period 1 IS written when period 2 starts
// ---------------------------------------------------------------------------
TEST_F(LoadFileTest, TwoPeriodFileWritesPeriod1) {
    writeInputFile({
        makeOrderLine(T1,   1, symbol, "BUY", "NEW", 50.0, 100),
        makeOrderLine(T1+1, 2, symbol, "SELL","NEW", 55.0, 200),
        // Period 2 order triggers flush of period 1
        makeOrderLine(T2,   3, symbol, "BUY", "NEW", 52.0, 300),
    });
    runLoad();
    // Period 1 should have been written
    EXPECT_FALSE(timeIdx->isEmpty());
    auto out = captureQuery(T1 + 1);
    EXPECT_NE(out.find("50"), std::string::npos);
    EXPECT_NE(out.find("55"), std::string::npos);
}

TEST_F(LoadFileTest, TwoPeriodFileWritesBothPeriods) {
    writeInputFile({
        makeOrderLine(T1, 1, symbol, "BUY", "NEW", 50.0, 100),
        makeOrderLine(T2, 2, symbol, "BUY", "NEW", 52.0, 300),
    });
    runLoad();
    auto out = captureQuery(T2);
    EXPECT_NE(out.find("52"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Three-period file: periods 1 and 2 are written; period 3 is lost (bug)
// ---------------------------------------------------------------------------
TEST_F(LoadFileTest, ThreePeriodFileWritesFirstTwoPeriods) {
    writeInputFile({
        makeOrderLine(T1,   1, symbol, "BUY", "NEW", 50.0, 100),
        makeOrderLine(T2,   2, symbol, "BUY", "NEW", 51.0, 200),
        makeOrderLine(T3,   3, symbol, "BUY", "NEW", 52.0, 300),
    });
    runLoad();

    // Period 1 written (flushed when T2 order arrives)
    auto out1 = captureQuery(T1);
    EXPECT_NE(out1.find("50"), std::string::npos);

    // Period 2 written (flushed when T3 order arrives)
    auto out2 = captureQuery(T2);
    EXPECT_NE(out2.find("51"), std::string::npos);
}

// ---------------------------------------------------------------------------
// TRADE in period 1 updates lastTrade stored in the file
// ---------------------------------------------------------------------------
TEST_F(LoadFileTest, TradeInPeriod1UpdatesLastTrade) {
    writeInputFile({
        makeOrderLine(T1,   1, symbol, "BUY",  "NEW",   50.0, 100),
        makeOrderLine(T1+1, 2, symbol, "BUY",  "TRADE", 50.0, 30),
        // Period 2 triggers flush of period 1
        makeOrderLine(T2,   3, symbol, "BUY",  "NEW",   55.0, 50),
    });
    runLoad();

    // Read the period-1 file directly to check lastTrade
    auto nearestTs = timeIdx->findNearestIndexPrior(T1);
    ASSERT_NE(nearestTs, static_cast<uint64_t>(-1));
    storage::Reader reader;
    reader.loadData(storage::getFileName(rootDir, symbol, nearestTs));
    auto lt = reader.getLastTrade();
    EXPECT_EQ(lt.qty, 30u);
    EXPECT_DOUBLE_EQ(lt.price, 50.0);
}

// ---------------------------------------------------------------------------
// CANCEL reduces qty in loaded period
// ---------------------------------------------------------------------------
TEST_F(LoadFileTest, CancelReducesQtyInFile) {
    writeInputFile({
        makeOrderLine(T1,   1, symbol, "BUY", "NEW",    50.0, 100),
        makeOrderLine(T1+1, 2, symbol, "BUY", "CANCEL", 50.0, 40),
        makeOrderLine(T2,   3, symbol, "BUY", "NEW",    55.0, 50),
    });
    runLoad();

    auto out = captureQuery(T1 + 1);
    // Remaining qty at 50.0 = 60
    EXPECT_NE(out.find("60"), std::string::npos);
}
