cmake_minimum_required(VERSION 3.10)

# set the project name
project(zone_benchmarks)

# specify the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

message(STATUS "We require git.")
find_package(Git REQUIRED)
CPMAddPackage(
    NAME GTest
    GITHUB_REPOSITORY NLnetLabs/simdzone
    VERSION 0.1.0
)

# add the executable
add_subdirectory(benchmarks)
