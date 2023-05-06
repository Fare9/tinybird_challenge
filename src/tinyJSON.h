#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>
#include <variant>
#include <utility>
#include <numeric>

typedef std::string ColumnString;
typedef uint32_t ColumnUInt32;
typedef int32_t ColumnInt32;
typedef uint64_t ColumnUInt64;
typedef int64_t ColumnInt64;
typedef float ColumnFloat32;
static_assert(sizeof(ColumnFloat32) == 4);
typedef double ColumnFloat64;
static_assert(sizeof(ColumnFloat64) == 8);

typedef std::unordered_map<std::string, std::vector<std::uint32_t>> HashIndex;

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

    struct getSymbol
    {
        const ColumnString & operator()(Row const & r) { return r.symbol; }
    };
    struct getDate
    {
        const ColumnString & operator()(Row const & r) { return r.date; }
    };
    struct getHigh
    {
        const ColumnFloat32 & operator()(Row const & r) { return r.high; }
    };
    struct getLow
    {
        const ColumnFloat32 & operator()(Row const & r) { return r.low; }
    };
    struct getVolume
    {
        const ColumnUInt32 & operator()(Row const & r) { return r.volume; }
    };
};


/// Structure that will contain the data stored as columnar data
struct ColumnarDatabase
{
public:
    std::uint32_t index = 0;
    /// string values can include a cache value,
    /// since looking for strings is an expensive operation
    HashIndex symbol_m;
    HashIndex date_m;
    /// for avoiding repetition of strings in symbol_v
    /// and date_v, let's keep a cache of strings, and
    /// to make faster the lookup, use a hash as key
    /// symbol_v and date_v will just keep a pointer
    /// to the stored strings.
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
        symbol_m[symbol].push_back(index);
        date_m[date].push_back(index);

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
        
        high_v.push_back(high);
        low_v.push_back(low);
        open_v.push_back(open);
        close_v.push_back(close);
        close_adjusted_v.push_back(close_adjusted);
        volume_v.push_back(volume);
        split_coefficient_v.push_back(split_coefficient);
        index++;
    }

    /// I offer a similar interface to the row storage
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

    
};


struct Storage
{
    ColumnarDatabase c_data;

    std::vector<Row> data;

    Storage() = default;
    explicit Storage(std::vector<Row> && data_) : data(data_){};
    /// Constructor for operations with ColumnarDatabase object
    explicit Storage(ColumnarDatabase && c_data_) : c_data(c_data_){};

    size_t count() const { return data.size(); }

    template <typename columnType, typename Op>
    int64_t countEquals(const columnType & value) const
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        int64_t count = 0;
        for (auto const & e : data)
        {
            if (Op()(e) == value)
                count++;
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs] -> " << count
                  << " elements found\n";

        return count;
    }
    /// countEquals for columnar data
    template <typename columnType, typename Op>
    int64_t countEqualsColumn(const columnType & value) const;

    template <typename columnType, typename Op>
    Storage filterEquals(const columnType & value) const
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        std::vector<Row> new_data;

        for (auto const & e : data)
        {
            if (Op()(e) == value)
                new_data.push_back(e);
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs] -> "
                  << new_data.size() << " elements left\n";

        return Storage(std::move(new_data));
    }

    /// filterEquals for columnar data
    template <typename columnType, typename Op>
    Storage filterEqualsColumn(const columnType & value) const;

    template <typename columnType, typename Op>
    columnType getMax() const
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        columnType max = std::numeric_limits<columnType>::lowest();

        for (auto const & e : data)
        {
            columnType v = Op()(e);
            if (v >= max)
                max = v;
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs] -> MAX: " << max << "\n";

        return max;
    }

    /// getMax for columnar data
    template <typename columnType, typename Op>
    columnType getMaxColumn() const
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        columnType max = std::numeric_limits<columnType>::lowest();

        const auto &vect = Op()(c_data);

        auto it = max_element(vect.begin(), vect.end());

        if (it != vect.end())
            max = *it;

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs] -> MAX: " << max << "\n";

        return max;
    }

    template <typename columnType, typename Op>
    columnType getMin() const
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        columnType min = std::numeric_limits<columnType>::max();

        for (auto const & e : data)
        {
            columnType v = Op()(e);
            if (v <= min)
                min = v;
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs] -> MIN: " << min << "\n";

        return min;
    }
    
    /// getMin for columnar data
    template <typename columnType, typename Op>
    columnType getMinColumn() const
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        columnType max = std::numeric_limits<columnType>::lowest();

        const auto &vect = Op()(c_data);

        auto it = min_element(vect.begin(), vect.end());

        if (it != vect.end())
            max = *it;

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs] -> MAX: " << max << "\n";

        return max;
    }

    template <typename columnType, typename Op>
    columnType getSum() const
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        columnType sum{0};

        for (auto const & e : data)
        {
            sum += Op()(e);
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs] -> SUM: " << sum << "\n";

        return sum;
    }

    /// getSum for columnar data
    template <typename columnType, typename Op>
    columnType getSumColumn() const
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        columnType sum{0};

        const auto &vect = Op()(c_data);

        sum = std::accumulate(vect.begin(), vect.end(), sum);

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
                  << "[µs] -> SUM: " << sum << "\n";

        return sum;
    }
};