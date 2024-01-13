cmake_minimum_required(VERSION 3.14)

project(
    Utily
    VERSION 0.0.1
    LANGUAGES CXX
)

file(GLOB_RECURSE UTILY_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE UTILY_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
file(GLOB_RECURSE UTILY_BENCHMARK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/benchmark/*.cpp")

add_library(Utily_Utily STATIC ${UTILY_LIB_SOURCES})
add_library(Utily::Utily ALIAS Utily_Utily)

# So Visual Studio can have the format/tidy info. 
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/.clang-format DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

set_property(TARGET Utily_Utily PROPERTY EXPORT_NAME Utily)

target_compile_features(Utily_Utily PUBLIC cxx_std_20)
target_include_directories(Utily_Utily PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Test Setup
find_package(GTest CONFIG) 
if(GTest_FOUND)
    add_executable(UtilyTest ${UTILY_TEST_SOURCES})
    target_link_libraries(UtilyTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main Utily::Utily)
    enable_testing()
    add_test(NAME AllTestsInUtilyTest COMMAND UtilyTest)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
    message(STATUS "Package \"GTest\" not found, not building tests.")
endif()
    
# Benchmark Setup    
find_package(benchmark CONFIG)
if(benchmark_FOUND)
    add_executable(UtilyBenchmark ${UTILY_BENCHMARK_SOURCES})
    target_link_libraries(UtilyBenchmark PRIVATE benchmark::benchmark benchmark::benchmark_main Utily::Utily)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
    message(STATUS "Package \"benchmark\" not found, not building benchmarks.")
endif()
