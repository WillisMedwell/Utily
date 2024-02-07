cmake_minimum_required(VERSION 3.11)

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

if(DEFINED EMSCRIPTEN)
    message(STATUS "For Emscripten SIMD use flags: \"-msimd128 -mrelaxed-simd -msse -msse2 -msse3 -msse4.1\"")
elseif(NOT MSVC)
    target_compile_options(Utily_Utily PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wcast-align" "-Wcast-qual" "-Wctor-dtor-privacy" "-Wformat=2" "-Winit-self" "-Wmissing-declarations" "-Wmissing-include-dirs" "-Wold-style-cast" "-Woverloaded-virtual" "-Wredundant-decls" "-Wshadow" "-Wsign-conversion" "-Wsign-promo" "-Wstrict-overflow=5" "-Wundef" "-Wno-unused" "-Wconversion" "-Wsign-compare")
    message(STATUS "For Native SIMD use flags: \"-march=native\"")
endif()

set_property(TARGET Utily_Utily PROPERTY EXPORT_NAME Utily)

target_compile_features(Utily_Utily PUBLIC cxx_std_20)
target_include_directories(Utily_Utily PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(BUILD_UTILY_TESTS "Build Utily test suite" OFF)
option(BUILD_UTILY_BENCHMARKS "Build Utily benchmarks" OFF)

if(BUILD_UTILY_TESTS OR BUILD_UTILY_BENCHMARKS)
    include(FetchContent)
endif()

# Test Setup
if(BUILD_UTILY_TESTS)
    find_package(GTest CONFIG) 
    if(NOT GTest_FOUND)
        message(STATUS "Downloading GTest...")
        FetchContent_Declare(
            GTest
            GIT_REPOSITORY https://github.com/google/googletest
            GIT_TAG main
            GIT_SHALLOW FALSE
        )
        FetchContent_MakeAvailable(GTest)
    endif()

    add_executable(UtilyTest ${UTILY_TEST_SOURCES})
    if(DEFINED EMSCRIPTEN)
        target_compile_options(UtilyTest PRIVATE "-Wdeprecated-declarations")
        target_compile_options(UtilyTest PRIVATE -msimd128 -mrelaxed-simd -msse -msse2 -msse3 -msse4.1)
        set(CMAKE_EXECUTABLE_SUFFIX ".html")
    elseif(NOT MSVC)
        target_compile_options(UtilyTest PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wcast-align" "-Wcast-qual" "-Wctor-dtor-privacy" "-Wformat=2" "-Winit-self" "-Wmissing-declarations" "-Wmissing-include-dirs" "-Wold-style-cast" "-Woverloaded-virtual" "-Wredundant-decls" "-Wshadow" "-Wsign-conversion" "-Wsign-promo" "-Wstrict-overflow=5" "-Wundef" "-Wno-unused" "-Wconversion" "-Wsign-compare")
        target_compile_options(UtilyTest PRIVATE -march=native)
        # target_compile_options(UtilyTest PRIVATE -g -fsanitize=address -fno-omit-frame-pointer)
        # target_link_options(UtilyTest PRIVATE -g -fsanitize=address -fno-omit-frame-pointer)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(UtilyTest PRIVATE -Wuseless-cast)
        endif()
    endif()
    

    target_link_libraries(UtilyTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main Utily::Utily)
    enable_testing()
    add_test(NAME AllTestsInUtilyTest COMMAND UtilyTest)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Benchmark Setup    
if(BUILD_UTILY_BENCHMARKS)
    find_package(benchmark CONFIG)
    if(NOT benchmark_FOUND)
        message(STATUS "Downloading Benchmark...")

        FetchContent_Declare(
            benchmark
            GIT_REPOSITORY https://github.com/google/benchmark
            GIT_TAG main
            GIT_SHALLOW FALSE
        )
        
        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable Google Benchmark tests" FORCE)
        FetchContent_MakeAvailable(benchmark)
    endif()
    add_executable(UtilyBenchmark ${UTILY_BENCHMARK_SOURCES})
    

    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    if(DEFINED EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
        # Preload files...
        file(GLOB_RECURSE ASSET_FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/*")
        set(PRELOAD_FILES "")
        foreach(FILE ${ASSET_FILES})
            file(RELATIVE_PATH REL_FILE ${CMAKE_SOURCE_DIR} ${FILE})
            set(PRELOAD_FILES "${PRELOAD_FILES} --preload-file ${REL_FILE}")
        endforeach()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PRELOAD_FILES} -Wno-unused-command-line-argument")
        target_compile_options(UtilyBenchmark PRIVATE -Wno-deprecated-declarations -Wno-unused-command-line-argument)
        target_compile_options(UtilyBenchmark PRIVATE -msimd128 -mrelaxed-simd -msse -msse2 -msse3 -msse4.1)
    elseif(NOT MSVC)
        target_compile_options(UtilyBenchmark PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wcast-align" "-Wcast-qual" "-Wctor-dtor-privacy" "-Wformat=2" "-Winit-self" "-Wmissing-declarations" "-Wmissing-include-dirs" "-Wold-style-cast" "-Woverloaded-virtual" "-Wredundant-decls" "-Wshadow" "-Wsign-conversion" "-Wsign-promo" "-Wstrict-overflow=5" "-Wswitch-default" "-Wundef" "-Wno-unused" "-Wconversion" "-Wsign-compare")
        target_compile_options(UtilyBenchmark PRIVATE -mtune=native)
        target_compile_options(UtilyBenchmark PRIVATE
            $<$<CONFIG:Release>:-O3 -DNDEBUG>
        )
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(UtilyBenchmark PRIVATE -Wuseless-cast)
        endif()
    endif()
    target_link_libraries(UtilyBenchmark PRIVATE benchmark::benchmark benchmark::benchmark_main Utily::Utily)
endif()

