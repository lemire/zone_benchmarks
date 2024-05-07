add_executable(bench bench.cpp)
target_link_libraries(bench PRIVATE zscanner)
target_include_directories(bench PRIVATE .)
