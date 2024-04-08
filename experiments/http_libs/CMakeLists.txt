cmake_minimum_required (VERSION 3.13)
project (http_tests)

set (CMAKE_CXX_STANDARD 20)


# experiment dependecies
#=---------------------------------------------------------
CPMAddPackage(NAME cpp_httplib
    GIT_REPOSITORY "https://github.com/yhirose/cpp-httplib"
    GIT_TAG b8bafbc29129a9f12e58032e608b51996219d6f5
    DOWNLOAD_ONLY TRUE
)

find_package(cpprestsdk REQUIRED)

add_executable(http_test 
    main.cpp
)

target_link_libraries(http_test PUBLIC cpprestsdk::cpprest crypto ssl cpr::cpr)
target_link_libraries(http_test PUBLIC libcurl)

target_include_directories(http_test PUBLIC ${cpp_httplib_SOURCE_DIR})
target_include_directories(http_test PUBLIC ${rapidjson_SOURCE_DIR}/include)
target_include_directories(http_test PUBLIC ${curl_lib_SOURCE_DIR}/include)
target_include_directories(http_test PUBLIC ${libcpr_p_SOURCE_DIR}/include)
