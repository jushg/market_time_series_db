#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "storages.hpp"
#include "TempDirFixture.hpp"
#include "OrderDataBuilder.hpp"

class TimeIndexTest : public TempDirFixture {
protected:
    // Create an empty .dat file in the symbol directory with the given timestamp
    // (TimeIndex only looks at filenames, not content).
    void touchDataFile(uint64_t ts) {
        auto symDir = storage::getSymbolDirectory(rootDir, symbol);
        std::filesystem::create_directories(symDir);
        std::ofstream f(symDir + "/" + std::to_string(ts) + ".dat",
                        std::ios::out | std::ios::binary);
        // Write minimal valid file so reader doesn't complain if accessed
        storage_model::Metadata meta{0, 0, 0};
        storage_model::LastTradeRecord lt;
        f.write(reinterpret_cast<char*>(&meta), sizeof(meta));
        f.write(reinterpret_cast<char*>(&lt), sizeof(lt));
        f.close();
    }
};

// ---------------------------------------------------------------------------
// New index on empty directory is empty
// ---------------------------------------------------------------------------
TEST_F(TimeIndexTest, IsEmptyOnNewDir) {
    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);
    EXPECT_TRUE(idx->isEmpty());
}

// ---------------------------------------------------------------------------
// Index picks up pre-existing data files
// ---------------------------------------------------------------------------
TEST_F(TimeIndexTest, LoadsExistingDataFiles) {
    // Create files BEFORE constructing TimeIndex
    touchDataFile(T1);
    touchDataFile(T2);

    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);
    EXPECT_FALSE(idx->isEmpty());
}

// ---------------------------------------------------------------------------
// findNearestIndexAfter
// ---------------------------------------------------------------------------
TEST_F(TimeIndexTest, FindNearestIndexAfterExactMatch) {
    touchDataFile(T1);
    touchDataFile(T2);
    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);

    EXPECT_EQ(idx->findNearestIndexAfter(T1), T1);
    EXPECT_EQ(idx->findNearestIndexAfter(T2), T2);
}

TEST_F(TimeIndexTest, FindNearestIndexAfterBetweenValues) {
    touchDataFile(T1);
    touchDataFile(T2);
    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);

    // Between T1 and T2 → should return T2
    EXPECT_EQ(idx->findNearestIndexAfter(T1 + 1), T2);
}

TEST_F(TimeIndexTest, FindNearestIndexAfterBeyondAll) {
    touchDataFile(T1);
    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);

    // After all elements → returns (uint64_t)-1
    EXPECT_EQ(idx->findNearestIndexAfter(T1 + 1), static_cast<uint64_t>(-1));
}

// ---------------------------------------------------------------------------
// findNearestIndexPrior
// ---------------------------------------------------------------------------
TEST_F(TimeIndexTest, FindNearestIndexPriorExactMatch) {
    touchDataFile(T1);
    touchDataFile(T2);
    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);

    EXPECT_EQ(idx->findNearestIndexPrior(T1), T1);
    EXPECT_EQ(idx->findNearestIndexPrior(T2), T2);
}

TEST_F(TimeIndexTest, FindNearestIndexPriorBetweenValues) {
    touchDataFile(T1);
    touchDataFile(T2);
    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);

    // Between T1 and T2 → should return T1
    EXPECT_EQ(idx->findNearestIndexPrior(T1 + 1), T1);
}

TEST_F(TimeIndexTest, FindNearestIndexPriorBeforeAll) {
    touchDataFile(T1);
    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);

    // Before all elements → returns (uint64_t)-1
    EXPECT_EQ(idx->findNearestIndexPrior(T1 - 1), static_cast<uint64_t>(-1));
}

// ---------------------------------------------------------------------------
// findIndexesCoverRange
// ---------------------------------------------------------------------------
TEST_F(TimeIndexTest, FindIndexesCoverRangeAllIncluded) {
    touchDataFile(T1);
    touchDataFile(T2);
    touchDataFile(T3);
    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);

    auto result = idx->findIndexesCoverRange(T1, T3);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], T1);
    EXPECT_EQ(result[1], T2);
    EXPECT_EQ(result[2], T3);
}

TEST_F(TimeIndexTest, FindIndexesCoverRangeSubset) {
    touchDataFile(T1);
    touchDataFile(T2);
    touchDataFile(T3);
    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);

    auto result = idx->findIndexesCoverRange(T1, T2);
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], T1);
    EXPECT_EQ(result[1], T2);
}

TEST_F(TimeIndexTest, FindIndexesCoverRangeStartBeforeFirst) {
    touchDataFile(T2);
    touchDataFile(T3);
    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);

    // Start before T2; should still include T2
    auto result = idx->findIndexesCoverRange(T1, T3);
    ASSERT_GE(result.size(), 1u);
    EXPECT_EQ(result.back(), T3);
}

// ---------------------------------------------------------------------------
// Persistence roundtrip: destroy and reconstruct TimeIndex
// ---------------------------------------------------------------------------
TEST_F(TimeIndexTest, PersistenceRoundtrip) {
    {
        // First: create index with data files, let destructor persist idx.dat
        touchDataFile(T1);
        touchDataFile(T2);
        auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);
        EXPECT_FALSE(idx->isEmpty());
        // idx destroyed here → writes idx.dat
    }

    // Second: new TimeIndex reads from idx.dat (binary path in loadIdx)
    auto idx2 = std::make_shared<storage::TimeIndex>(symbol, rootDir);
    EXPECT_FALSE(idx2->isEmpty());
    EXPECT_EQ(idx2->findNearestIndexPrior(T1), T1);
    EXPECT_EQ(idx2->findNearestIndexPrior(T2), T2);
}

// ---------------------------------------------------------------------------
// loadIdxFromFile (called via constructor when idx.dat absent)
// ---------------------------------------------------------------------------
TEST_F(TimeIndexTest, ReloadAfterAddingFile) {
    auto idx = std::make_shared<storage::TimeIndex>(symbol, rootDir);
    EXPECT_TRUE(idx->isEmpty());

    // Simulate writing a new data file externally then reloading
    touchDataFile(T1);
    idx->loadIdxFromFile();   // safe: idx.dat not written yet (destructor hasn't run)
    EXPECT_FALSE(idx->isEmpty());
    EXPECT_EQ(idx->findNearestIndexPrior(T1), T1);
}
