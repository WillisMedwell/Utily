cmake_minimum_required(VERSION 3.14)
project(Utily)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE UTILY_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
file(GLOB_RECURSE UTILY_BENCHMARK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/benchmark/*.cpp")

# So Visual Studio can have the format/tidy info. 
file(COPY ${CMAKE_SOURCE_DIR}/.clang-format DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/.clang-tidy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -g")
endif()

# UtilyTest Setup
add_executable(UtilyTest ${UTILY_TEST_SOURCES})
target_include_directories(UtilyTest PRIVATE ${CMAKE_SOURCE_DIR}/include)
find_package(GTest CONFIG REQUIRED) 
target_link_libraries(UtilyTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
enable_testing()
add_test(NAME AllTestsInUtilyTest COMMAND UtilyTest)
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# UtilyBenchmark Setup
add_executable(UtilyBenchmark ${UTILY_BENCHMARK_SOURCES})
target_include_directories(UtilyBenchmark PRIVATE ${CMAKE_SOURCE_DIR}/include)
find_package(benchmark CONFIG REQUIRED)
target_link_libraries(UtilyBenchmark PRIVATE benchmark::benchmark benchmark::benchmark_main)
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

