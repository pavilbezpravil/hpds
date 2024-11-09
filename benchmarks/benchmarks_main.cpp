#include <benchmark/benchmark.h>

#include "Helpers.h"

int Fibonacci(int v) {
   return v < 2 ? v : Fibonacci(v - 1) + Fibonacci(v - 2);
}

void BM_Fibonacci(benchmark::State& state) {
   for (auto _ : state) {
      int res = Fibonacci((int)state.range(0));
      benchmark::DoNotOptimize(res);
   }
}
BENCHMARK(BM_Fibonacci)->DenseRange(5, 20);

void BM_BusyWaitForNanoseconds(benchmark::State& state) {
   for (auto _ : state) {
      BusyWaitForNanoseconds((int)state.range(0));
   }
}
BENCHMARK(BM_BusyWaitForNanoseconds)->Range(1, 10000);

void BM_EmulateWork(benchmark::State& state) {
   for (auto _ : state) {
      EmulateWork((int)state.range(0));
   }
}
BENCHMARK(BM_EmulateWork)->RangeMultiplier(2)->Range(1, 1000000);

BENCHMARK_MAIN();
