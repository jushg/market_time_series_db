#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "../models/order.cpp"
#include "../models/order_book.cpp"
#include "../storage/writer.cpp"
#include "../utils/utils.cpp"

void insertInputFile(std::string& fileName) {
    std::ifstream handler(fileName);
    std::string line;

    uint64_t curPeriodStart = -1;

    order_book::OrderBook book;
    std::string symbol;
    std::vector<order::OrderData> curPeriodOrders;
    while (std::getline(handler, line)) {
        auto newOrder = parseInputToOrderData(line);
        symbol = newOrder.symbol; // Todo: Change this
        if (curPeriodStart == -1) curPeriodStart = newOrder.timestamp;
        else if(!isSamePeriod(curPeriodStart, newOrder.timestamp)) {
            auto fileName = getFileName(curPeriodStart,symbol);
            std::ofstream handler(fileName, std::ios::out | std::ios::binary);
            writer::write(handler, book, curPeriodOrders);
            curPeriodStart = newOrder.timestamp;
            curPeriodOrders.clear();
        }
        book.add(newOrder);
        curPeriodOrders.push_back(std::move(newOrder));
    }
}


order::OrderData parseInputToOrderData(std::string& inputLine) {
    // Todo:: move const away
    std::string BUY_INPUT_STR = "BUY";
    std::string CANCEL_INPUT_STR = "CANCEL";
    std::string TRADE_INPUT_STR = "TRADE";


    std::stringstream stream(inputLine);
    std::istream_iterator<std::string> begin(stream);
    std::istream_iterator<std::string> end;
    std::vector<std::string> fields(begin, end);

    order::Side side = (fields[3] == BUY_INPUT_STR) ? order::Side::BUY : order::Side::SELL;

    order::Category category;
    if (fields[4] == CANCEL_INPUT_STR) category = order::Category::CANCEL;
    else if (fields[4] == TRADE_INPUT_STR)category =  order::Category::TRADE;
    else category = order::Category::NEW;

    order::OrderData order(fields[2],
                std::stoull(fields[0]),
                std::stoull(fields[1]),
                side,
                category,
                std::stoull(fields[6]),
                atof(fields[5].c_str()));

    return order;
}