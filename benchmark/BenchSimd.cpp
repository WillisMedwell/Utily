#include "Utily/Utily.hpp"


#include <benchmark/benchmark.h>
#include "Utily/Utily.hpp"

#include <ranges>
#include <algorithm>
#include <array>

constexpr static std::string_view LONG_STRING = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaz";
//constexpr static std::string_view LONG_STRING = "this is my testing zone";

const static auto iota_view = std::views::iota(0, 512);
const static std::vector<int32_t> NUMS = { iota_view.begin(), iota_view.end() };

static void BM_Utily_Simd_find(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = Utily::Simd::find(LONG_STRING.begin(), LONG_STRING.end(), 'z');
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK(BM_Utily_Simd_find);

static void BM_Utily_Simd_find_contiguous_char_128(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = Utily::Simd::find_contiguous_char_128(LONG_STRING.data(), (LONG_STRING.data() + LONG_STRING.size()), 'z');
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK(BM_Utily_Simd_find_contiguous_char_128);

static void BM_Utily_Simd_find_contiguous_char_256(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = Utily::Simd::find_contiguous_char_256(LONG_STRING.data(), (LONG_STRING.data() + LONG_STRING.size()), 'z');
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK(BM_Utily_Simd_find_contiguous_char_256);

static void BM_Utily_Simd_find_contiguous_char_512(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = Utily::Simd::find_contiguous_char_512(LONG_STRING.data(), (LONG_STRING.data() + LONG_STRING.size()), 'z');
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK(BM_Utily_Simd_find_contiguous_char_512);


static void BM_Std_find_char(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = std::find(LONG_STRING.begin(), LONG_STRING.end(), 'z');
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK(BM_Std_find_char);

static void BM_Utily_Simd_find_contiguous_chars_128(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = Utily::Simd::find_contiguous_chars_128(LONG_STRING.data(), (LONG_STRING.data() + LONG_STRING.size()), std::to_array({'z', 'o', 'n'}));
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK(BM_Utily_Simd_find_contiguous_chars_128);

static void BM_Std_find_chars(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = LONG_STRING.find_first_of("zon");
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK(BM_Std_find_chars);



static void BM_Utily_Simd_find_contiguous_int32_128(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = Utily::Simd::find_contiguous_int32_128(NUMS.data(), (NUMS.data() + NUMS.size()), NUMS.back());
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK( BM_Utily_Simd_find_contiguous_int32_128);



static void BM_Utily_Simd_find_contiguous_int32_512(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = Utily::Simd::find_contiguous_int32_512(NUMS.data(), (NUMS.data() + NUMS.size()), NUMS.back());
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK( BM_Utily_Simd_find_contiguous_int32_512);



static void BM_Utily_Std_find_int32(benchmark::State& state) {
    for (auto _ : state) {
        auto iter = std::find(NUMS.data(), (NUMS.data() + NUMS.size()), NUMS.back());
        benchmark::DoNotOptimize(iter); 
    }
}
BENCHMARK(BM_Utily_Std_find_int32);
