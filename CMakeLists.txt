cmake_minimum_required(VERSION 2.8) 

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-unknown-pragmas" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

project(eventually)

set(EVENTUALLY_SOURCE_DIR "src/eventually")
file(GLOB_RECURSE EVENTUALLY_SOURCES
    "${EVENTUALLY_SOURCE_DIR}/*.cpp"
)

add_library(eventually ${EVENTUALLY_SOURCES})

set(EVENTUALLY_TESTS_DIR "test/tests")
file(GLOB_RECURSE EVENTUALLY_TESTS
    "${EVENTUALLY_TESTS_DIR}/*.cpp"
)

add_subdirectory("lib/test/gtest")
enable_testing()
include_directories("src" "test")
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
add_executable(runUnitTests ${EVENTUALLY_TESTS})
target_link_libraries(runUnitTests eventually gtest gtest_main)
add_test(runUnitTests runUnitTests)
