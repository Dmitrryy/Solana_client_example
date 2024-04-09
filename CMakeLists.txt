cmake_minimum_required (VERSION 3.13)
project (task)

set (CMAKE_CXX_STANDARD 20)

include(cmake/CPM.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# common dependecies
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
CPMAddPackage(NAME libcpr_p
    GIT_REPOSITORY "https://github.com/libcpr/cpr"
    GIT_TAG 11e595428cb824bf24b9d70b286a69e9568b92da
    OPTIONS
    "BUILD_SHARED_LIBS OFF"
)
# CPMAddPackage(NAME one_tbb
#     GIT_REPOSITORY "https://github.com/oneapi-src/oneTBB"
#     GIT_TAG 54a22a627c942311bc5b4bdf58803af397ded2f5
#     OPTIONS
#     "TBB_TEST OFF"
# )


option(BUILD_EXPERIMENTS "Build binaries with experiments (require additional dependdencies)" OFF)
if (BUILD_EXPERIMENTS)
    add_subdirectory(experiments)
endif()


# tasks
add_subdirectory(task1)
add_subdirectory(task2_3)
