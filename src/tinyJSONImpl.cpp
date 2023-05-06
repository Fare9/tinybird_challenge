/**
*   The next file cover all the implementations of different template
*   and template specializations.
*/
#include "tinyJSON.h"


template <typename columnType, typename Op>
int64_t Storage::countEqualsColumn(const columnType & value) const
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    const auto &vect = Op()(c_data);

    int64_t count = std::count(vect.begin(), vect.end(), value);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs] -> " << count
                << " elements found\n";

    return count;
}

/**
*   Both getSymbol and getDate are a cache map with the string values, making
*   the search operations on them easier and with a complexity of O(1)
*/

template <>
int64_t Storage::countEqualsColumn<ColumnString, ColumnarDatabase::getSymbol>(const ColumnString & value) const
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    const auto &map = ColumnarDatabase::getSymbol()(c_data);

    int64_t count = map.at(value).size();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs] -> " << count
                << " elements found\n";

    return count;
}

template <>
int64_t Storage::countEqualsColumn<ColumnString, ColumnarDatabase::getDate>(const ColumnString & value) const
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    const auto &map = ColumnarDatabase::getDate()(c_data);

    int64_t count = map.at(value).size();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs] -> " << count
                << " elements found\n";

    return count;
}

template <typename columnType, typename Op>
Storage Storage::filterEqualsColumn(const columnType & value) const
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    ColumnarDatabase new_data;

    const auto &vect = Op()(c_data);

    auto it = std::find(vect.begin(), vect.end(), value);

    while (it != vect.end())
    {
        auto index = std::distance(vect.begin(), it++);

        new_data.addData(*c_data.symbol_v[index], *c_data.date_v[index], c_data.high_v[index], c_data.low_v[index], c_data.open_v[index], c_data.close_v[index], c_data.close_adjusted_v[index], c_data.volume_v[index], c_data.split_coefficient_v[index]);

        it = std::find(it, vect.end(), value);
    }
    
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs] -> "
                << new_data.symbol_v.size() << " elements left\n";

    return Storage(std::move(new_data));
}

template<>
Storage Storage::filterEqualsColumn<ColumnString, ColumnarDatabase::getSymbol>(const ColumnString & value) const
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    ColumnarDatabase new_data;

    const auto &map = ColumnarDatabase::getSymbol()(c_data);

    for (auto index : map.at(value))
    {
        new_data.addData(*c_data.symbol_v[index], *c_data.date_v[index], c_data.high_v[index], c_data.low_v[index], c_data.open_v[index], c_data.close_v[index], c_data.close_adjusted_v[index], c_data.volume_v[index], c_data.split_coefficient_v[index]);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs] -> "
                << new_data.symbol_v.size() << " elements left\n";

    return Storage(std::move(new_data));
}

template<>
Storage Storage::filterEqualsColumn<ColumnString, ColumnarDatabase::getDate>(const ColumnString & value) const
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    ColumnarDatabase new_data;

    const auto &map = ColumnarDatabase::getDate()(c_data);

    for (auto index : map.at(value))
    {
        new_data.addData(*c_data.symbol_v[index], *c_data.date_v[index], c_data.high_v[index], c_data.low_v[index], c_data.open_v[index], c_data.close_v[index], c_data.close_adjusted_v[index], c_data.volume_v[index], c_data.split_coefficient_v[index]);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << __func__ << " : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs] -> "
                << new_data.symbol_v.size() << " elements left\n";

    return Storage(std::move(new_data));
}