# market-data-warehouse




## Data Engine Query 

### Insert

Support 2 different data insertion methods:

- Load from file for a single symbol:
LOAD <file_path> <symbol>

- Insert a single row:

INSERT <symbol> <epoch> <id> <side:BUY/SELL> <category:NEW/TRADE/CANCEL> <price> <quantity>


## Query

Support a variety of query methods:

Each line will always contains the <symbol> and <epoch>

<data> that can be included:
buy1, buy2, buy3, buy4, buy5, sell1, sell2, sell3, sell4, sell5, last_trade

If want to query all, use ALL for <data>

- Query single symbol:
    - At only 1 timestamp:
        FROM <symbol> AT <epoch> QUERY <data>

    - With a range:
        FROM <symbol> RANGE <start> <end> <granularity> QUERY <data>

- Query multiple symbols
    - At only 1 timestamp:
        FROM_MULTIPLE <symbol_1> <symbol_2> ... <symbol_n> AT <epoch> QUERY <data>

    - With a range
        FROM_MULTIPLE <symbol_1> <symbol_2> ... <symbol_n> RANGE <start> <end> <granularity> QUERY <data>

## Delete/ Update

As with this proof of concept development, the


