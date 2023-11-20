
#include "../../include/client.hpp"


QueryConfig getQueryConfig(size_t startFrom, std::vector<std::string>& fields) {
    if(fields[startFrom] == ALL) return QueryConfig();
    std::vector<std::string> dataFields(fields.begin()+startFrom, fields.end());
    return QueryConfig(dataFields);
}

bool parseAndExecute(std::string& cmd, EngineData& engineData) {
    std::stringstream stream(cmd);
    // std::istream_iterator<std::string> begin(stream);
    // std::istream_iterator<std::string> end;
    std::vector<std::string> fields((std::istream_iterator<std::string>(stream)), std::istream_iterator<std::string>());

    if (fields.size() < 1) {
        return true;
    }

    if(fields[0] == QUIT) return false;

    // INSERT <symbol> <epoch> <id> <side:BUY/SELL> <category:NEW/TRADE/CANCEL> <price> <quantity>
    if (fields[0] == INSERT) {
        // if(fields.size() != 8) return {ExitCommand()};
        auto symbol = fields[1];
        auto timestamp = std::stoull(fields[2]);
        auto id = std::stoull(fields[3]);
        auto side = fields[4] == "BUY"? model::Side::BUY : model::Side::SELL;
        auto cat = fields[5] == "NEW"? model::Category::NEW : 
                    fields[5] == "TRADE"? model::Category::TRADE :model::Category::CANCEL;
        auto price =  atof(fields[6].c_str());
        auto qty = std::stoull(fields[7]);
        auto cmd = InsertEntryCommand(
                    model::OrderData(symbol,timestamp,id, side, cat, qty,price),
                    CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)});
        cmd.execute();

    //LOAD <file_path> <symbol>
    } 
    else if(fields[0] == LOAD) {
        auto symbol = fields[1];
        auto cmd = LoadFileCommand(fields[2],CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)});
        cmd.execute();

    } else if(fields[0] == FROM_QUERY_SINGLE) {
        auto symbol = fields[1];

        //FROM <symbol> AT <epoch> QUERY <data>
        if(fields[2] == AT) {
            auto timestamp = std::stoull(fields[3]);

            auto cmd = QuerySingleCommand(
                timestamp,
                CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)},
                getQueryConfig(5,fields)
            );
            cmd.execute();
        // FROM <symbol> RANGE <start> <end> <granularity> QUERY <data>
        } else if(fields[2] == RANGE) {
            auto start = std::stoull(fields[3]);
            auto end = std::stoull(fields[4]);
            auto gra = std::stoull(fields[5]);
            auto cmd = QueryRangeCommand(
                start,
                end, 
                gra,
                CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)},
                getQueryConfig(7,fields)
            );
            cmd.execute();
        }
    } else if(fields[0] == FROM_QUERY_MULTIPLE) {
        std::vector<std::string> symbols;
        int idx = 1;
        while(fields[idx] != AT && fields[idx] != RANGE) symbols.push_back(fields[idx++]);

        // FROM_MULTIPLE <symbol_1> <symbol_2> ... <symbol_n> AT <epoch> QUERY <data>
        if(fields[idx] == AT) {
            auto timestamp = std::stoull(fields[idx+1]);
            for(auto symbol: symbols) {
                auto cmd = QuerySingleCommand(
                            timestamp,
                            CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)},
                            getQueryConfig(idx+3,fields)
                        );
                cmd.execute();
            }

        // FROM_MULTIPLE <symbol_1> <symbol_2> ... <symbol_n> RANGE <start> <end> <granularity> QUERY <data>
        } else if(fields[2] == RANGE) {
            auto start = std::stoull(fields[idx+1]);
            auto end = std::stoull(fields[idx+2]);
            auto gra = std::stoull(fields[idx+3]);
            for(auto symbol: symbols) {
                auto cmd = QueryRangeCommand(
                                start, 
                                end, 
                                gra,
                                CommonConfig{engineData.rootDir,symbol, engineData.findIdx(symbol)},
                                 getQueryConfig(idx+5,fields)
                            );
                cmd.execute();
            }
        }
    }  
    else if(fields[0] == NUKE) {
        NukeDataCommand cmd(engineData.rootDir);
        cmd.execute();
    }
    return true;
}