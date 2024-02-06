// #define UTY_NO_SIMD
// #define UTY_USE_SIMD_128
// #define UTY_USE_SIMD_256
// #define UTY_USE_SIMD_512

#include "Utily/Utily.hpp"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <array>
#include <numeric>
#include <ranges>

#if 1 

auto all_a(size_t max) -> std::string {
    std::string v;
    v.resize(max);
    std::fill(v.begin(), v.end(), 'a');
    v.append("a");
    v.append("stringer");
    return v;
}
const static std::string LONG_STRING = all_a(10000);

auto iota(size_t max) -> std::vector<int32_t> {
    std::vector<int32_t> v;
    v.resize(max);
    std::iota(v.begin(), v.end(), 0);
    return v;
}
const static std::vector<int32_t> NUMS = iota(10000);


static void BM_Uty_find_char(benchmark::State& state) {
    for (auto _ : state) {
        volatile auto iter = Utily::Simd128::Char::find(LONG_STRING.data(), LONG_STRING.size(), 'z');
        benchmark::DoNotOptimize(iter);
    }
}
BENCHMARK(BM_Uty_find_char);

static void BM_Std_find_char(benchmark::State& state) {
    for (auto _ : state) {
        volatile auto iter = std::find(LONG_STRING.begin(), LONG_STRING.end(), 'z');
        benchmark::DoNotOptimize(iter);
    }
}
BENCHMARK(BM_Std_find_char);


static void BM_Uty_find_first_of_char(benchmark::State& state) {
    const auto data = std::to_array({ 'z', 'o', 'n' });
    for (auto _ : state) {
        volatile auto iter = Utily::Simd128::Char::find_first_of(LONG_STRING.data(), LONG_STRING.size(), data.data(), data.size());
        benchmark::DoNotOptimize(iter);
    }
}
BENCHMARK(BM_Uty_find_first_of_char);

static void BM_Std_find_first_of_chars(benchmark::State& state) {
    const auto data = std::to_array({ 'z', 'o', 'n' });
    for (auto _ : state) {
        volatile auto iter = std::find_first_of(LONG_STRING.begin(), LONG_STRING.end(), data.begin(), data.end());
        benchmark::DoNotOptimize(iter);
    }
}
BENCHMARK(BM_Std_find_first_of_chars);

static void BM_Uty_search_char_4letters(benchmark::State& state) {
    std::string_view find = "stri";
    for (auto _ : state) {
        volatile auto iter = Utily::Simd128::Char::search(
            LONG_STRING.data(),
            LONG_STRING.size(),
            find.data(),
            find.size());
        benchmark::DoNotOptimize(iter);
    }
}
BENCHMARK(BM_Uty_search_char_4letters);

static void BM_Std_search_char_4letters(benchmark::State& state) {
    std::string_view find = "stri";
    for (auto _ : state) {
        volatile auto iter = std::search(
            LONG_STRING.begin(),
            LONG_STRING.end(),
            find.begin(),
            find.end());
        benchmark::DoNotOptimize(iter);
    }
}
BENCHMARK(BM_Std_search_char_4letters);


static void BM_Uty_search_char_8letters(benchmark::State& state) {
   std::string_view find = "stringer";
   for (auto _ : state) {
       volatile auto iter = Utily::Simd128::Char::search(
           LONG_STRING.data(),
           LONG_STRING.size(),
           find.data(),
           find.size());
       benchmark::DoNotOptimize(iter);
   }
}
BENCHMARK(BM_Uty_search_char_8letters);

static void BM_Std_search_char_8letters(benchmark::State& state) {
    std::string_view find = "stringer";
    for (auto _ : state) {
        volatile auto iter = std::search(
            LONG_STRING.begin(),
            LONG_STRING.end(),
            find.begin(),
            find.end());
        benchmark::DoNotOptimize(iter);
    }
}
BENCHMARK(BM_Std_search_char_8letters);

#endif