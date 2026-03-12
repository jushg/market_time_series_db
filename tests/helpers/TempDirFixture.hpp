#pragma once
#include <gtest/gtest.h>
#include <filesystem>
#include <string>

// RAII temp-dir fixture. Each test gets an isolated directory under /tmp.
class TempDirFixture : public ::testing::Test {
protected:
    std::filesystem::path tempDir;
    std::string rootDir;
    std::string symbol = "TEST";

    void SetUp() override {
        auto testName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        tempDir = std::filesystem::temp_directory_path()
                  / ("mkt_test_" + std::string(testName));
        std::filesystem::remove_all(tempDir);   // clean any leftover from previous run
        std::filesystem::create_directories(tempDir);
        rootDir = tempDir.string();
    }

    void TearDown() override {
        std::filesystem::remove_all(tempDir);
    }
};
