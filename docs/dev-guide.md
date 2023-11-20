# Technical Documentation

In this part, I will explain the architecture of this implementation, some major design choices, current limitations as well as future improvements


## Contents
- [Key Assumptions](#key-assumptions)
- [Storage Structure](#storage-structure)
- [Architecture](#architecture)
- [Limitations](#limitations)
- [Improvements](#improvements)


## Key assumptions 
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

## Storage structure

- To optimise for read speed, I structure the storage to store data based on the symbol, with each symbol directory storing order specifically belong to it in multiple files.
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

### File structure

- The underlying file have 4 parts:
  - **Metadata:** stores the sizes of the other sections
   - **Last Trade:** stores the last trade details for stats aggregation
  - **Base state:** stores the aggregated order-book before this epoch window start time
  - **Orders:** stores the fine-grained individual order details within the file's epoch window (without aggregation)

### Auxilary Index

An index is built per symbol subdirectory to faciliate fast query and modifying operations. This index store the current existing epoch window start time. The index is currently rebuilt after each insertion operation to maintain the treee logic

## Architecture

Adhering to good software engineering practice, this implementation is split into different components with minimal interactions with each other to prevent tight coupling. The components from top to bottom are:

### Application/Client

The interactive shell environment, responsible for init the storage directory as needed, and parsing the user input following the Query language specify in the section above.

### Command

The individual command that is parse from the user input, calling other components below it to execute the logic

### Shared 

Contains the operations functions shared by all the commands to perform certain action. This can be categories as the same layer as command, but I split this out to maintain readability.


### Model

Consist of 2 subcomponents: `Model` and `Storage Model`, the first one is use for normal application logic, and the second one is space optimised storage model for storing data to the disk. There are also utils functions for us to map between these 2 as needed.

### Storage

Include a "dumb" `Writer` and `Reader`, use as a facade for us to interact with std::filesystem. Does not contain business logic.
Also consisit of functions for getting file name and directory in a pre-defined format. Contains the `index tree` class , use to load from and write to persistence storage the current state of the data subdirectory.


## Limitations

- The current implementation read data file to memory and write to disk for each epoch windows. This will some performance issue if the data are very concentrated in one window. For larger scale implementation, more metrics need to be taken into consideration beside epoch window, to decide how to split each data chunk.
- For each query, the current logic get the nearest file timestamp and try to aggregate from there. This would be a significant bottleneck for data that are concentrade in one window, as mentioned above.
- Index is currently rebuilt per each insertion operations, this is a potential bottleneck as the index size grow larger
- Currently the engine is single-threaded. Although there are some exception (Redis), usually this would not scale well as the data read/write volume increase to production scale.

## Improvements

- As mentioned above, this is a crude implementation taking inspiration from Prometheus and InfluxDB, as well as some other key value database. However, due to time constraints, many assisting data structures and algorithms use in the above database are not incoorporated here.
- Most important are `Sorted String Table` used for storing and retrieving data from persistence storage, as well as allow us to easily compact historic data. This would allow us to be able to support efficient update/delete operation for specific rows.
- `Segment Tree` should also be incoporated as a in-memory storage for inserting, as this would allow us to have dynamic file size depend on the orders number and independent of window size, which would allow us to solve Limitation 1 and 2 mentioned above
- To allow for scaling of this data engine to multiple concurrent window, locking mechanism should be put in place for each file to prevent the race condition when multiple instances try to write to the same files.
