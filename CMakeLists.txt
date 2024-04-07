cmake_minimum_required (VERSION 3.13)
project (task)

set (CMAKE_CXX_STANDARD 20)

include(cmake/CPM.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

option(BUILD_EXPERIMENTS "Build binaries with experiments (require additional dependdencies)" OFF)
# if (BUILD_EXPERIMENTS)
    add_subdirectory(experiments)
# endif()


# tasks
add_subdirectory(task1)
# add_subdirectory(task2)
# add_subdirectory(task3)
