
#include "Utily/Utily.hpp"
#include <benchmark/benchmark.h>

#include <ranges>

#if 1

// Apparently gcc's implementation of std::views::split is not compatible with const strings... 
static std::string LONG_STRING = "The quick brown fox jumps over the lazy dog while the energetic and spirited puppies, intrigued by the sudden burst of activity, start chasing their own tails, causing a delightful commotion in the serene and peaceful meadow that has, until now, been undisturbed by such playful endeavors";

static void BM_Utily_SplitByElement(benchmark::State& state) {
    for (auto _ : state) {
        for (auto word : Utily::split(LONG_STRING, ' ')) {
            benchmark::DoNotOptimize(word);
        }
    }
}
BENCHMARK(BM_Utily_SplitByElement);

static void BM_Utily_SplitByElements(benchmark::State& state) {
    for (auto _ : state) {
        for (auto word : Utily::split(LONG_STRING, ' ', 't')) {
            benchmark::DoNotOptimize(word);
        }
    }
}
BENCHMARK(BM_Utily_SplitByElements);

static void BM_Std_SplitByElement(benchmark::State& state) {
    for (auto _ : state) {
        auto splitter = LONG_STRING | std::views::split(' ');
        for (auto&& word : splitter) {
            benchmark::DoNotOptimize(word);
        }
    }
}
BENCHMARK(BM_Std_SplitByElement);

#endif