cmake_minimum_required(VERSION 3.0.0)
project(TinyJSON)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_BUILD_TYPE Release)

SET(CMAKE_CXX_FLAGS "-Wall -Wextra")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

include(FetchContent)

FetchContent_Declare(simdjson
        GIT_REPOSITORY https://github.com/simdjson/simdjson.git
        GIT_TAG  tags/v1.0.0
        GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(simdjson)

file(GLOB tinyJSON_sources
    "src/*.h"
    "src/*.cpp")

add_executable(tinyJSON ${tinyJSON_sources})
target_link_libraries(tinyJSON simdjson)
