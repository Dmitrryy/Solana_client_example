cmake_minimum_required (VERSION 3.13)
project (task1)

set (CMAKE_CXX_STANDARD 20)

add_executable(task1 
    task1.cpp
)

target_link_libraries(task1 PUBLIC crypto ssl cpr::cpr)
target_include_directories(task1 PUBLIC ${rapidjson_SOURCE_DIR}/include)
target_include_directories(task1 PUBLIC ${curl_lib_SOURCE_DIR}/include)
