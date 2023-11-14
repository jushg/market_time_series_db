# Time-Series Data Engine for Market Orders

This is a simplfied implementation of a data warehouse to support efficient researches and simulations with large data volumes, written using C++17 with no external libraries. The internal implementation of data compression and structure for efficient time-based queries take heavy inspiration from other popular time-series database soultions: InfluxDB and Prometheus.


## Contents
- [Quick start](#quick-start)
- [Engine Client Usage](#engine-client)

## Compiler and architecure support

This project uses `CMake`, and have been tested for stable run on `MacOS` and `Linux(Ubuntu)`.
Every commit in main is tested with `GCC` on armv6, armv7, aarch64, riscv64, s390x, and ppc64le.
Require C++17 for the usage of `std::filesystem` 
Extensive usage of C++11 and C++14 features for effcient codebase

## Quick Start


First, clone and build

```sh
# Whereever you cloned this codebase to
$ cmake .
$ make 
```

Then, start the interactive engine client

```sh
$ ./data_engine
```

On startup, you will be ask to input a path for the persistence storage. Simply type `0` to use the default location, which is in the local directory.


## Engine Client Usage 


This will give the basic instructions on how to test this client implementations

### Insertion

```
[Load from file for a single symbol]
        LOAD <file_path> <symbol>

[Insert a single order]
        INSERT <symbol> <epoch> <id> <side:BUY/SELL> <category:NEW/TRADE/CANCEL> <price> <quantity>

[insert one order into database - use engine directly for file ingestions]
        INSERT <symbol> AT <epoch> VALUES <id> <side:BUY/SELL> <category:NEW/TRADE/CANCEL> <price> <quantity>

[delete order by epoch-id pair for a symbol]
        DELETE <symbol> WITH <epoch> <id>

[update order by epoch-id pair for a symbol with other values]
        UPDATE <symbol> WITH <epoch> <id> VALUES <side:BUY/SELL> <category:NEW/TRADE/CANCEL> <price> <quantity>
```

### Query

The engine client currently supports a variety of query methods

```
[Query single symbol - At only 1 timestamp]
        FROM <symbol> AT <epoch> QUERY <data>

[Query single symbol - Within a range and granularity]
        FROM <symbol> RANGE <start> <end> <granularity> QUERY <data>

[Query multiple symbols - At only 1 timestamp]
        FROM_MULTIPLE <symbol_1> <symbol_2> ... <symbol_n> AT <epoch> QUERY <data>

[Query multiple symbols - Within a range and granularity]
        FROM_MULTIPLE <symbol_1> <symbol_2> ... <symbol_n> RANGE <start> <end> <granularity> QUERY <data>
```

Each result line will always contains the `symbol` and `epoch`
Data fields supported for fine-grained queries: `buy1`, `buy2`, `buy3`, `buy4`, `buy5`, `sell1`, `sell2`, `sell3`, `sell4`, `sell5`, `last_trade`

To query all, type `ALL` after `QUERY`, for example: `FROM SCH AT 1609722840752518773 QUERY ALL`

### Delete/ Update

For this implementation, I don't support fine-grained delete and update of order rows, as there is no efficient way for me to efficiently re-compressed the data after update/delete

### Other Commands

```
[Exit the engine client gracefully]
        QUIT

[Delete all data inside the storage]
        NUKE_DB
```

# Technical Documentation

In this part, I will explain the architecture of this implementation, some major design choices, current limitations as well as future improvements

### Key assumptions with the implementation
- Data from files and trading systems would most likely arrive linearly forward, and modification (insert/update/delete) in the middle of the data is not common
- Write-intensive and fast read for data analytics purpose
- Single instance running only
- All data for file ingestions will be cleaned and sorted according to epoch prior to ingestion, and will only data for one symbol in one file

### Supported order types
- **NEW:** A new order entry that is available to trade, to be insert to the book.
- **TRADE:** Match and remove the quantiy of order of the price that match from the book (only on the affected buy/sell side).
- **CANCEL:** Removes a quantity of order from the book.

### Insertions
- Insertions can be made individually for each entry, or by loading file
- For file ingestion, the file should be cleaned follow this format:
```
epoch  |  id  |  symbol  |  side(BUY/SELL)  |  category(NEW/TRADE/CANCEL)  |  price  |  quantity
```
- Ingestions are well-optimized if the orders are being appended on top of temporally previous orders.
- Middle of the row insertion would be possible, but less efficient as we need to merge different data chunks

### Queries
- Supports singular epoch and range with granularity queries, with custom specified fields if needed
- Can query multiple symbol at the same time (underlying it will call the query for one symbol at a time)

### Storage structure

- To optimise for read speed, I structure the storage to store data based on the symbol, with each symbol directory storing order specifically belong to it in multiple files.
- An auxilary index is store to facillitate fast navigation through the data tree.
- Each file stores the order data for a given epoch window. Each file will be named by the epoch at which the window starts. For instance:
```
persistence_storage/
  SCH/      <-- Symbol 
    123.dat <-- File for an epoch window
    234.dat
    ...
    idx.dat <--- Index for each symbol
  STH/
    100.dat
    200.dat
    ...
    idx.dat
```

