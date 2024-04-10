cmake_minimum_required (VERSION 3.13)
project (task1)

set (CMAKE_CXX_STANDARD 20)

add_executable(task2 
    task2.cpp
)

target_link_libraries(task2 PUBLIC crypto ssl cpr::cpr TBB::tbb)
target_include_directories(task2 PUBLIC ${rapidjson_SOURCE_DIR}/include)
target_include_directories(task2 PUBLIC ${curl_lib_SOURCE_DIR}/include)
