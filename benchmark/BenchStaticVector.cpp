
#include "Utily/Utily.hpp"
#include <benchmark/benchmark.h>


constexpr int N = 100;
using T = std::vector<int>;

static void BM_StaticVector_Construct(benchmark::State& state) {
    for (auto _ : state) {
        Utily::StaticVector<T, N> v;
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_StaticVector_Construct);



static void BM_StaticVector_PushBack(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        Utily::StaticVector<T, N> v;
        state.ResumeTiming();
        for (auto i = 0; i < N; ++i) {
            v.push_back(T{ i });
        }
    }
}
BENCHMARK(BM_StaticVector_PushBack);

static void BM_StaticVector_Iterate(benchmark::State& state) {
    Utily::StaticVector<T, N> v;
    for (auto i = 0; i < N; ++i) {
        v.push_back({ i });
    }
    for (auto _ : state) {
        for (auto& x : v) {
            benchmark::DoNotOptimize(x);
        }
    }
}
BENCHMARK(BM_StaticVector_Iterate);

static void BM_Vector_Construct(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<T> v;
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_Vector_Construct);

static void BM_Vector_PushBack(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<T> v;
        state.ResumeTiming();
        for (auto i = 0; i < N; ++i) {
            v.push_back(T { i });
        }
    }
}
BENCHMARK(BM_Vector_PushBack);

static void BM_Vector_Iterate(benchmark::State& state) {
    std::vector<T> v;
    for (auto i = 0; i < N; ++i) {
        v.push_back(T { i });
    }
    for (auto _ : state) {
        for (auto& x : v) {
            benchmark::DoNotOptimize(x);
        }
    }
}
BENCHMARK(BM_Vector_Iterate);

static void BM_Array_Construct(benchmark::State& state) {
    for (auto _ : state) {
        std::array<T, N> a;
        benchmark::DoNotOptimize(a);
    }
}
BENCHMARK(BM_Array_Construct);

static void BM_Array_Assign(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        std::array<T, N> a;
        state.ResumeTiming();
        for (auto i = 0; i < N; ++i) {
            a[i] = T{i};
        }
    }
}
BENCHMARK(BM_Array_Assign);

static void BM_Array_Iterate(benchmark::State& state) {
    std::array<T, N> a;
    for (auto i = 0; i < N; ++i) {
        a[i] = T{i};
    }
    for (auto _ : state) {
        for (auto& x : a) {
            benchmark::DoNotOptimize(x);
        }
    }
}
BENCHMARK(BM_Array_Iterate);