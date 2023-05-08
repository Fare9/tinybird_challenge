# Solution

This file will explain the solution to the cpp test provided as challenge by the company *tinybird*.

## Goal of the challenge

As stated in the README.md file provided with the challenge, I have to read and understand the code given, this code is a sketch of what is doable with any other analytical framework, once after reading and understanding the code, I have to apply different changes to reach one of the proposed goals (see README.md). Mostly these changes will become an speedup on the queries applied or an improvement on the memory usage by the program.

## Solution applied

The code given is a storage mechanism for saving the data from a JSON file with a specific format, and it allows a user doing different queries in the data to obtain different information. The first thing I realized was that the storage mechanism is implemented *row-oriented*, this means that the Storage class will keep a vector of *rows*, being this *row* a C++ structure that keeps one variable for each value from the JSON file.

```cpp
struct Row
{
public:
    ColumnString symbol;
    ColumnString date;

    ColumnFloat32 high;
    ColumnFloat32 low;
    ColumnFloat32 open;
    ColumnFloat32 close;
    ColumnFloat32 close_adjusted;

    ColumnUInt32 volume;
    ColumnFloat32 split_coefficient;
    ...

struct Storage
{
    std::vector<Row> data;
    ...
```

This implementation makes easy the operations of writing, and also the operations where we read all the fields of the row at the same time. But, it becomes costly when we make queries only in specific columns, since all the time I want to query for a value in a column, the Storage class will go over all the rows, even if some of the columns are not necessary. While the implementation of *std::vector* gives a nice performance for accessing data, the way data is stored, where same data is not stored together, it makes searching process expensive.

After reading [1], it is easy to understand the main differences between row-oriented and column-oriented storage, and the benefits from each one. Although I read that one of the downsides from column-oriented is the complexity of the queries that need to be applied, the benefits offered regarding data retrieving are enough for changing the storage type used, and implement it as column-oriented.

### Column-oriented storage

My implementation modifies the way data is stored in memory, while in the original implementation a row-oriented storage is provided, I have modified for a column-oriented storage. The changes are provided in a structure called *ColumnarDatabase*, and the following variables are declared as part of the structure (please refer to the source code for more information):

```cpp
struct ColumnarDatabase
{
public:
    std::uint32_t index = 0;
    HashIndex symbol_m;
    HashIndex date_m;
    std::unordered_map<ColumnString, ColumnString> strings;

    std::vector<ColumnString *> symbol_v;
    std::vector<ColumnString *> date_v;

    std::vector<ColumnFloat32> high_v;
    std::vector<ColumnFloat32> low_v;
    std::vector<ColumnFloat32> open_v;
    std::vector<ColumnFloat32> close_v;
    std::vector<ColumnFloat32> close_adjusted_v;

    std::vector<ColumnUInt32> volume_v;
    std::vector<ColumnFloat32> split_coefficient_v;
```

This time, the data instead of stored as a *vector* of *rows*, I have a structure that contains a *vector* (implementation of a column) for each one of the different values from the JSON file. Since this time I do not have one variable for each value, I provide a function to add data into the different structures *addData* (explained later).

For allowing a comparison between the previous implementation, and the current one, instead of replacing the code, I included my code as part of the *Storage* structure and I provide a similar interface that user can use. For example a function called *countEquals*, for using my storage class, will have the name *countEqualsColumn*, and the function can be called in a very similar way:

```cpp
/// calling provided function
database.countEquals<ColumnString, Row::getSymbol>("AMD");
/// calling new function
database.countEqualsColumn<ColumnString, ColumnarDatabase::getSymbol>("AMD");
```

Also, its internal implementation is similar, since the structure I created offers structures for retrieving its data in a way the class *row* provided:

```cpp
struct getSymbol
{
    const std::unordered_map<std::string, std::vector<std::uint32_t>> & operator()(ColumnarDatabase const & r) { return r.symbol_m; }
};
struct getDate
{
    const std::unordered_map<std::string, std::vector<std::uint32_t>> & operator()(ColumnarDatabase const & r) { return r.date_m; }
};
struct getHigh
{
    const std::vector<ColumnFloat32> & operator()(ColumnarDatabase const & r) { return r.high_v; }
};
struct getLow
{
    const std::vector<ColumnFloat32> & operator()(ColumnarDatabase const & r) { return r.low_v; }
};
struct getVolume
{
    const std::vector<ColumnUInt32> & operator()(ColumnarDatabase const & r) { return r.volume_v; }
};
```

What I have changed is the logic behind the querying functions, most of them make use of for-range loops, since each iteration needs to work with an object from the *row* structure, and although for-range loops have a good performance, in my case I have made use of different utilities from the standard library of C++, since the data in my structure is stored as vectors, and the code from the standard libraries have been carefully tested and highly optimized.

### Quick data access

In opposite to row-oriented storage, in column-oriented each column can be searched separately, and lookups on vectors are already fast and optimized. While this is true for columns that store things like numbers, it's different for the strings since string comparison has a cost equals to the length of the string. For that reason, using the class *std::unordered_map* from C++ I implemented a compression mechanism to store the indexes where a string can be found for two different values (*symbol* and *date*). In this way, whenever the user wants to access data by one of these values, the access has a complexity of O(1), also since the strings are hashed, the key size is equal to the output of the hash.

```cpp
typedef std::unordered_map<std::string, std::vector<std::uint32_t>> HashIndex;

struct ColumnarDatabase
{
public:
    ...
    HashIndex symbol_m;
    HashIndex date_m;
    ...

...

template <>
int64_t Storage::countEqualsColumn<ColumnString, ColumnarDatabase::getSymbol>(const ColumnString & value) const
{
    ...
    const auto &map = ColumnarDatabase::getSymbol()(c_data);
    int64_t count = map.at(value).size();
    ...
}

template <>
int64_t Storage::countEqualsColumn<ColumnString, ColumnarDatabase::getDate>(const ColumnString & value) const
{
    ...
    const auto &map = ColumnarDatabase::getDate()(c_data);
    int64_t count = map.at(value).size();
    ...
}

template<>
Storage Storage::filterEqualsColumn<ColumnString, ColumnarDatabase::getSymbol>(const ColumnString & value) const
{
    ...
    const auto &map = ColumnarDatabase::getSymbol()(c_data);

    for (auto index : map.at(value))
    {
        new_data.addData(...);
    }
    ...
}

template<>
Storage Storage::filterEqualsColumn<ColumnString, ColumnarDatabase::getDate>(const ColumnString & value) const
{
    ...
    const auto &map = ColumnarDatabase::getDate()(c_data);

    for (auto index : map.at(value))
    {
        new_data.addData(...);
    }
    ...
}
```

In the case of the other columns, the values are searched through the vector where the values are stored, while not so optimal like in this case, it is faster than the provided code.

### Memory usage

A problem I found with the implementation is that for each row from the JSON, two strings are stored, one for the *symbol* and one for the *date*, and the implementation did not care if the same string exist already in the Storage memory. For storing strings we will need minimum as many bytes as the length of the string. To avoid such a memory consumption, I have included a mechanism to store strings only once, keeping my solution optimal regarding memory usage.

```cpp
struct ColumnarDatabase
{
public:
    std::unordered_map<ColumnString, ColumnString> strings;

    std::vector<ColumnString *> symbol_v;
    std::vector<ColumnString *> date_v;

    ...

    void addData(ColumnString symbol, 
        ColumnString date, 
        ColumnFloat32 high, 
        ColumnFloat32 low, 
        ColumnFloat32 open, 
        ColumnFloat32 close, 
        ColumnFloat32 close_adjusted, 
        ColumnUInt32 volume,
        ColumnFloat32 split_coefficient)
    {
        ...
        auto look_up_fn = [&](ColumnString value)
        {
            auto it = strings.find(value);
            
            if (it == strings.end())
            {
                strings[value] = value;
                it = strings.find(value);
            }

            return &it->second;
        };
        
        symbol_v.push_back(look_up_fn(symbol));
        date_v.push_back(look_up_fn(date));
        ...

```

An *unordered_map* will be used to keep the stored strings, the benefit of using an *unordered_map*, in comparison with a *vector* is that when I lookup a value on it, the program instead of going over all the values from the vector, comparing strings, I only have to compare hash values, making the search more efficient, then the columns of *symbol* and *date* only store a pointer to the strings, and not the whole strings, being this a benefit for the cases where we can find a lot of repetition of string values.

### Implementation

For the implementation I only have made use of classes from the standard library, each one of the functions provided is implemented as a template, and for the cases of *symbol* and *date* values, an specialization of my templates for making use of the benefits provided by the *unordered_map*.

As I previosly said, instead of deleting the given implementation, I have kept it together with mine, so it is possible to do a comparison between both implementations on runtime.

## Testing

Since I provide the output from both implementations, it is possible to test the binary only compiling it and running it. I have tested it modifying the number of simulated data provided to the program, being presented here the next cases: x1, x10, x100 and x200.

* x1:

```console
$ ./tinyJSON ../stock_prices.json 
countEquals : 3669[µs] -> 4544 elements found
countEqualsColumn : 1[µs] -> 4544 elements found
filterEquals : 7233[µs] -> 965 elements left
getMax : 1[µs] -> MAX: 1179.86
filterEqualsColumn : 385[µs] -> 965 elements left
getMaxColumn : 0[µs] -> MAX: 1179.86
filterEquals : 8368[µs] -> 3439 elements left
filterEquals : 8[µs] -> 1 elements left
getMin : 0[µs] -> MIN: 31.4
filterEqualsColumn : 1513[µs] -> 3439 elements left
filterEqualsColumn : 2[µs] -> 1 elements left
getMinColumn : 0[µs] -> MAX: 31.4
filterEquals : 6953[µs] -> 362 elements left
getSum : 0[µs] -> SUM: 270787430
filterEqualsColumn : 103[µs] -> 362 elements left
getSumColumn : 0[µs] -> SUM: 270787430
```

* x10:

```console
$ ./tinyJSON ../stock_prices.json 
countEquals : 34719[µs] -> 45440 elements found
countEqualsColumn : 0[µs] -> 45440 elements found
filterEquals : 66483[µs] -> 9650 elements left
getMax : 46[µs] -> MAX: 1179.86
filterEqualsColumn : 1311[µs] -> 9650 elements left
getMaxColumn : 9[µs] -> MAX: 1179.86
filterEquals : 67250[µs] -> 34390 elements left
filterEquals : 211[µs] -> 10 elements left
getMin : 0[µs] -> MIN: 31.4
filterEqualsColumn : 4355[µs] -> 34390 elements left
filterEqualsColumn : 9[µs] -> 10 elements left
getMinColumn : 0[µs] -> MAX: 31.4
filterEquals : 66891[µs] -> 3620 elements left
getSum : 1[µs] -> SUM: 2707874300
filterEqualsColumn : 382[µs] -> 3620 elements left
getSumColumn : 0[µs] -> SUM: 2707874300
```

* x100:

```console
$ ./tinyJSON ../stock_prices.json 
countEquals : 330902[µs] -> 454400 elements found
countEqualsColumn : 1[µs] -> 454400 elements found
filterEquals : 578134[µs] -> 96500 elements left
getMax : 448[µs] -> MAX: 1179.86
filterEqualsColumn : 6782[µs] -> 96500 elements left
getMaxColumn : 85[µs] -> MAX: 1179.86
filterEquals : 590744[µs] -> 343900 elements left
filterEquals : 2265[µs] -> 100 elements left
getMin : 0[µs] -> MIN: 31.4
filterEqualsColumn : 31161[µs] -> 343900 elements left
filterEqualsColumn : 21[µs] -> 100 elements left
getMinColumn : 0[µs] -> MAX: 31.4
filterEquals : 581711[µs] -> 36200 elements left
getSum : 100[µs] -> SUM: 27078743000
filterEqualsColumn : 2377[µs] -> 36200 elements left
getSumColumn : 5[µs] -> SUM: 27078743000
```

* x200:

```console
$ ./tinyJSON ../stock_prices.json 
countEquals : 658490[µs] -> 908800 elements found
countEqualsColumn : 1[µs] -> 908800 elements found
filterEquals : 990216[µs] -> 193000 elements left
getMax : 840[µs] -> MAX: 1179.86
filterEqualsColumn : 14856[µs] -> 193000 elements left
getMaxColumn : 172[µs] -> MAX: 1179.86
filterEquals : 980967[µs] -> 687800 elements left
filterEquals : 4360[µs] -> 200 elements left
getMin : 0[µs] -> MIN: 31.4
filterEqualsColumn : 52103[µs] -> 687800 elements left
filterEqualsColumn : 33[µs] -> 200 elements left
getMinColumn : 0[µs] -> MAX: 31.4
filterEquals : 941956[µs] -> 72400 elements left
getSum : 294[µs] -> SUM: 54157486000
filterEqualsColumn : 5682[µs] -> 72400 elements left
getSumColumn : 13[µs] -> SUM: 54157486000
```

## Last notes

In the current challenge, the implementation was specific for the format of the provided JSON, and that allowed me to apply the optimizations previously explained. In a real case the format would be specified by the user, and a more generic approach should be taken for storing the columns (e.g. for a user defined data, this should be decomposed on internal types). Also I used a very simple approach for storing integers, since I used the hash algorithm from *std::unordered_map*, but other approaches (e.g. storing strings in a dictionary, which is a similar approach to the one used in Apache Parquet) would have better performance.

[1]: https://www.geeksforgeeks.org/difference-between-row-oriented-and-column-oriented-data-stores-in-dbms/
