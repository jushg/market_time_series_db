#include "../../include/commands.hpp"

void InsertFileCommand::execute(std::string& rootDir, std::string& symbol, std::shared_ptr<storage::TimeIndex> timeIdx) {
    std::ifstream handler(inputFile);
    std::string line;

    uint64_t curPeriodStart = -1;

    model::OrderBook book;
    std::string symbol;
    std::vector<model::OrderData> curPeriodOrders;
    while (std::getline(handler, line)) {
        auto newOrder = parseInputToOrderData(line);
        symbol = newOrder.symbol; // Todo: Change this
        if (curPeriodStart == -1) curPeriodStart = newOrder.timestamp;
        else if(!isSamePeriod(curPeriodStart, newOrder.timestamp)) {
            auto fileName = storage::getSymbolDirectory(rootDir, symbol) + "/" + std::to_string(curPeriodStart);
            std::ofstream handler(fileName, std::ios::out | std::ios::binary);
            storage::write(handler, book, curPeriodOrders);
            curPeriodStart = newOrder.timestamp;
            curPeriodOrders.clear();
        }
        book.add(newOrder);
        curPeriodOrders.push_back(std::move(newOrder));
    }
}

model::OrderData InsertFileCommand::parseInputToOrderData(std::string& inputLine) {
    // Todo:: move const away
    std::string BUY_INPUT_STR = "BUY";
    std::string CANCEL_INPUT_STR = "CANCEL";
    std::string TRADE_INPUT_STR = "TRADE";


    std::stringstream stream(inputLine);
    std::istream_iterator<std::string> begin(stream);
    std::istream_iterator<std::string> end;
    std::vector<std::string> fields(begin, end);

    model::Side side = (fields[3] == BUY_INPUT_STR) ? model::Side::BUY : model::Side::SELL;

    model::Category category;
    if (fields[4] == CANCEL_INPUT_STR) category = model::Category::CANCEL;
    else if (fields[4] == TRADE_INPUT_STR)category =  model::Category::TRADE;
    else category = model::Category::NEW;

    model::OrderData order(fields[2],
                std::stoull(fields[0]),
                std::stoull(fields[1]),
                side,
                category,
                std::stoull(fields[6]),
                atof(fields[5].c_str()));

    return order;
}


