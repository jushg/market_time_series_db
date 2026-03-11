#include <gtest/gtest.h>
#include <filesystem>
#include "storages.hpp"
#include "TempDirFixture.hpp"

// ---------------------------------------------------------------------------
// getSymbolDirectory
// ---------------------------------------------------------------------------
TEST(FileManagerTest, GetSymbolDirectory) {
    EXPECT_EQ(storage::getSymbolDirectory("/data", "SYM"), "/data/SYM");
}

TEST(FileManagerTest, GetSymbolDirectoryEmptySymbol) {
    EXPECT_EQ(storage::getSymbolDirectory("/data", ""), "/data/");
}

// ---------------------------------------------------------------------------
// getFileName
// ---------------------------------------------------------------------------
TEST(FileManagerTest, GetFileName) {
    EXPECT_EQ(storage::getFileName("/data", "SYM", 12345ULL), "/data/SYM/12345.dat");
}

TEST(FileManagerTest, GetFileNameZeroTimestamp) {
    EXPECT_EQ(storage::getFileName("/data", "SYM", 0ULL), "/data/SYM/0.dat");
}

TEST(FileManagerTest, GetFileNameLargeTimestamp) {
    // Typical nanosecond epoch
    uint64_t ts = 1'000'000'000'000ULL;
    EXPECT_EQ(storage::getFileName("/data", "SYM", ts), "/data/SYM/1000000000000.dat");
}

// ---------------------------------------------------------------------------
// createSymbolDirectoryIfNotExist
// ---------------------------------------------------------------------------
class FileManagerDirTest : public TempDirFixture {};

TEST_F(FileManagerDirTest, CreatesDirectoryIfAbsent) {
    std::string sym = "NEWSYM";
    storage::createSymbolDirectoryIfNotExist(rootDir, sym);
    EXPECT_TRUE(std::filesystem::exists(rootDir + "/" + sym));
}

TEST_F(FileManagerDirTest, IdempotentOnExistingDirectory) {
    storage::createSymbolDirectoryIfNotExist(rootDir, symbol);
    // Calling again must not throw
    EXPECT_NO_THROW(storage::createSymbolDirectoryIfNotExist(rootDir, symbol));
    EXPECT_TRUE(std::filesystem::exists(rootDir + "/" + symbol));
}

TEST_F(FileManagerDirTest, CreatesRootDirIfAbsent) {
    // rootDir itself might not exist yet in a subdirectory scenario
    std::string newRoot = (tempDir / "newroot").string();
    EXPECT_FALSE(std::filesystem::exists(newRoot));
    storage::createSymbolDirectoryIfNotExist(newRoot, "SYM");
    EXPECT_TRUE(std::filesystem::exists(newRoot + "/SYM"));
}
