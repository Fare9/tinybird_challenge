#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

typedef std::string ColumnString;
typedef uint32_t ColumnUInt32;
typedef int32_t ColumnInt32;
typedef uint64_t ColumnUInt64;
typedef int64_t ColumnInt64;
typedef float ColumnFloat32;
static_assert(sizeof(ColumnFloat32) == 4);
typedef double ColumnFloat64;
static_assert(sizeof(ColumnFloat64) == 8);

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


struct Storage
{
    std::vector<Row> data;

    Storage() = default;
    explicit Storage(std::vector<Row> && data_) : data(data_){};

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
};
