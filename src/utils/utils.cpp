#include <string>
#include <stdint.h>
#include <vector>

constexpr uint64_t PERIOD = 1200000000000;

bool isSamePeriod(uint64_t startTimestamp, uint64_t curTimestamp) {
    return curTimestamp - startTimestamp <= PERIOD;
}

std::string getFileName(uint64_t startTimestamp, std::string symbol) {
    return symbol+"-"+std::to_string(startTimestamp);
}

std::vector<uint64_t> findIndexesInRange(const uint64_t startTime, const uint64_t endTime){
    return {startTime, endTime};
}

uint64_t findNearestIndexAfter(const uint64_t time){
    return time;
}

uint64_t findNearestIndexPrior(const uint64_t time) {
    return time;
}