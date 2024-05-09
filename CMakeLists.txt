cmake_minimum_required(VERSION 3.10)

# set the project name
project(zone_benchmarks)

# specify the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()
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
