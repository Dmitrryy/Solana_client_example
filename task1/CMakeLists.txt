cmake_minimum_required (VERSION 3.13)
project (task1)

set (CMAKE_CXX_STANDARD 20)

# dependecies
#=---------------------------------------------------------
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
    # DOWNLOAD_ONLY TRUE
)

add_executable(task1 
    task1.cpp
    ../src/SolanaAPI.cpp
)

target_link_libraries(task1 PUBLIC libcurl crypto ssl)
target_include_directories(task1 PUBLIC ${rapidjson_SOURCE_DIR}/include)
target_include_directories(task1 PUBLIC ${curl_lib_SOURCE_DIR}/include)
