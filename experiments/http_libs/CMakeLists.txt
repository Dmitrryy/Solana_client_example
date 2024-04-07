cmake_minimum_required (VERSION 3.13)
project (http_tests)

set (CMAKE_CXX_STANDARD 20)


# common dependecies
#=---------------------------------------------------------
# CPMAddPackage(NAME json_parser
#     GIT_REPOSITORY "https://github.com/nlohmann/json"
#     GIT_TAG 0457de21cffb298c22b629e538036bfeb96130b7
#     DOWNLOAD_ONLY TRUE
# )
CPMAddPackage(NAME rapidjson
    GIT_REPOSITORY "https://github.com/Tencent/rapidjson"
    GIT_TAG v1.1.0
    OPTIONS
    "RAPIDJSON_BUILD_DOC OFF"
    "RAPIDJSON_BUILD_EXAMPLES OFF"
    "RAPIDJSON_BUILD_TESTS OFF"
    "RAPIDJSON_BUILD_THIRDPARTY_GTESTS OFF"
)

CPMAddPackage(NAME curl_lib
    GIT_REPOSITORY "https://github.com/curl/curl"
    GIT_TAG dc178faee97360d99fff18beb89c21506a11014e
    OPTIONS
    "CMAKE_USE_LIBSSH2 OFF"
    "BUILD_CURL_EXE OFF"
    "BUILD_SHARED_LIBS OFF"
    "HTTP_ONLY ON"
    "BUILD_TESTING OFF"
)

CPMAddPackage(NAME cpp_httplib
    GIT_REPOSITORY "https://github.com/yhirose/cpp-httplib"
    GIT_TAG b8bafbc29129a9f12e58032e608b51996219d6f5
    DOWNLOAD_ONLY TRUE
)

find_package(cpprestsdk REQUIRED)

add_executable(http_test 
    main.cpp
)

target_link_libraries(http_test PUBLIC cpprestsdk::cpprest crypto ssl)
target_link_libraries(http_test PUBLIC libcurl)

target_include_directories(http_test PUBLIC ${cpp_httplib_SOURCE_DIR})
target_include_directories(http_test PUBLIC ${rapidjson_SOURCE_DIR}/include)
target_include_directories(http_test PUBLIC ${curl_lib_SOURCE_DIR}/include)
