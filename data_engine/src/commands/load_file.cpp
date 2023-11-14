#include "../../include/commands.hpp"

void LoadFileCommand::execute() {
    std::ifstream handler(inputFile);
    std::string line;
    uint64_t curPeriodStart = -1;
    model::OrderBook book;
    std::string symbol;
    std::vector<model::OrderData> curPeriodOrders;
    storage_model::LastTradeRecord lastTrade;
    while (std::getline(handler, line)) {
        auto newOrder = parseInputToOrderData(line);
        if (curPeriodStart == -1) {
            curPeriodStart = newOrder.timestamp;
            book = getOrderBookSnapshot(config, newOrder.timestamp);
        }

        else if(!isSamePeriod(curPeriodStart, newOrder.timestamp)) {
            if(config.timeIdx->findNearestIndexAfter(curPeriodStart) == -1) {
                auto fileName = storage::getSymbolDirectory(config.rootDir, symbol) + "/" + std::to_string(curPeriodStart);
                std::ofstream handler(fileName, std::ios::out | std::ios::binary);
                storage::write(handler, book, curPeriodOrders, lastTrade);
            } else {
                mergeStateAndWrite(config, curPeriodStart, curPeriodStart + PERIOD, curPeriodOrders);
                book = getOrderBookSnapshot(config, newOrder.timestamp);
            }
            curPeriodStart = newOrder.timestamp;
            curPeriodOrders.clear();
        }
        curPeriodOrders.push_back(std::move(newOrder));

    }
}

model::OrderData LoadFileCommand::parseInputToOrderData(std::string& inputLine) {
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

    return model::OrderData(fields[2], std::stoull(fields[0]), std::stoull(fields[1]),
                            side,category, std::stoull(fields[6]), atof(fields[5].c_str()));
}


