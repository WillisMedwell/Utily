#include "Utily/Utily.hpp"


#include <benchmark/benchmark.h>
#include "Utily/Utily.hpp"

#include <ranges>
#include <algorithm>

constexpr static std::string_view LONG_STRING = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaz";


static void BM_Utily_Simd_find(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = Utily::Simd::find(LONG_STRING.begin(), LONG_STRING.end(), 'z');
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK(BM_Utily_Simd_find);

static void BM_Utily_Std_find(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = std::find(LONG_STRING.begin(), LONG_STRING.end(), 'z');
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK(BM_Utily_Std_find);

