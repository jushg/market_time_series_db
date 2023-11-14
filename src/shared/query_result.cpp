#include "../../include/shared.hpp" 

QueryConfig::QueryConfig(){
    for(auto side: model::AllSide) {
        topOrderNeeded[side].set();
    }
    needLastTrade = true;
}

QueryConfig::QueryConfig(std::vector<std::string>& fields){
    for(auto side: model::AllSide) {
        topOrderNeeded[side].reset();
    }
    needLastTrade = false;

    for(auto field: fields) {
        if(field == "last_trade") needLastTrade = true;
        for(auto side: model::AllSide) {
            for(int i = 0; i < 5; i++) {
                std::string curField = side == model::Side::BUY? "buy":"sell";
                curField += std::to_string(i+1);
                if(field == curField)topOrderNeeded[side].set(i);
            }
        }
        
    }
}

void QueryConfig::printReturnFormat() {
    std::cout << "symbol, epoch, ";
    for(int i = 4; i >= 0; i--) {
        if(topOrderNeeded[model::Side::BUY][i]) std::cout <<"buy" << (i+1) << "q@buy" << (i+1) <<"p ";
    }
    std::cout << "X ";
    for(int i = 4; i >= 0; i--) {
        if(topOrderNeeded[model::Side::SELL][i]) std::cout <<"sell" << (i+1) << "q@sell" << (i+1) <<"p ";
    }
    std::cout << "X ";
    if(needLastTrade) {
        std::cout << "last trade price@last trade quantity";
    }
    std::cout << "\n";
    std::cout << "\n";
}


QueryResult::QueryResult(const model::Symbol& symbol, uint64_t timestamp, uint64_t lastTradeQty, double lastTradePrice, model::OrderBook& book): 
    symbol(symbol), timestamp(timestamp), lastTradeQty(lastTradeQty), lastTradePrice(lastTradePrice) {
        for(auto side: model::AllSide) {
            topOrders[side] =  book.getTop(side, numOrderToHold);
        }
}


void QueryResult::printEmptyQuery() { std::cout<< "Empty query as there is no data present"<<"\n"; }
void QueryResult::printInvalidQuery() { std::cout<< "Invalid query parameters"<<"\n"; }

void QueryResult::printResult(QueryConfig& config){
    std::cout << symbol << ", " << timestamp << ", ";
    for(auto side: model::AllSide) {
        for(int i = 4; i >= 0; i--) {
            if(config.topOrderNeeded[side][i]) {
                if(i >= topOrders[side].size()) std::cout <<"N.A@N.A ";
                else std::cout <<topOrders[side][i].second << "@" << topOrders[side][i].first <<" ";
            }
        }
        std::cout << "X ";
    }
    if(config.needLastTrade) {
        if(lastTradePrice < 0) std::cout << "N.A";
        else std::cout << lastTradePrice;
        std::cout << "@";
        if(lastTradeQty < 0) std::cout << "N.A ";
        else std::cout << lastTradeQty;
    }
    std::cout << "\n";
    std::cout << "\n";
}