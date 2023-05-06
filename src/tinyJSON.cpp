#include "tinyJSON.h"

#include "simdjson.h"
#include <iostream>
using namespace simdjson;


int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        std::cerr << "Missing argument (filename)\n";
        return 1;
    }

    Storage database;
    try
    {
        auto json = padded_string::load(argv[1]);
        ondemand::parser parser;
        ondemand::document_stream docs = parser.iterate_many(json.value());
        for (auto doc : docs)
        {
            Row e;
            e.symbol = doc["symbol"].get_string().value();
            e.date = doc["date"].get_string().value();

            e.high = static_cast<ColumnFloat32>(doc["high"].get_double());
            e.low = static_cast<ColumnFloat32>(doc["low"].get_double());
            e.open = static_cast<ColumnFloat32>(doc["open"].get_double());
            e.close = static_cast<ColumnFloat32>(doc["close"].get_double());
            e.close_adjusted = static_cast<ColumnFloat32>(doc["close_adjusted"].get_double());
            e.volume = static_cast<ColumnUInt32>(doc["volume"].get_int64_in_string());
            e.split_coefficient = static_cast<ColumnFloat32>(doc["split_coefficient"].get_double());

            // Simulate that we have 10x the data that we do
            for (size_t i = 0; i < 10; i++)
                database.data.push_back(e);
        }
    }
    catch (std::exception & e)
    {
        std::cerr << "Could not open and parse " << std::string(argv[1]) << ": " << e.what() << "\n";
        return 1;
    }

    size_t iterations = 1;
    for (size_t i = 0; i < iterations; i++)
    {
        database.countEquals<ColumnString, Row::getSymbol>("AMD");

        database.filterEquals<ColumnString, Row::getSymbol>("GOOG").getMax<ColumnFloat32, Row::getHigh>();

        database.filterEquals<ColumnString, Row::getSymbol>("ACAD")
            .filterEquals<ColumnString, Row::getDate>("2017-08-14")
            .getMin<ColumnFloat32, Row::getLow>();

        database.filterEquals<ColumnString, Row::getDate>("2017-08-14").getSum<ColumnUInt64, Row::getVolume>();
    }

    return 0;
}
