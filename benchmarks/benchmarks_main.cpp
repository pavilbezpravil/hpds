#include <benchmark/benchmark.h>

#include "Helpers.h"
#include "RingBuffer.h"

int Fibonacci(int v) {
   return v < 2 ? v : Fibonacci(v - 1) + Fibonacci(v - 2);
}

void BM_Fibonacci(benchmark::State& state) {
   for (auto _ : state) {
      int res = Fibonacci((int)state.range(0));
      benchmark::DoNotOptimize(res);
   }
}
BENCHMARK(BM_Fibonacci)->DenseRange(5, 10);

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

static void BM_RingBuffer_MultiThreaded(benchmark::State& state) {
   int ringBufferSize = (int)state.range(0);
   int count = (int)state.range(1);

   for (auto _ : state) {
      bool success = RingBufferMultiThreadTest(ringBufferSize, count);

      if (!success) {
         state.SkipWithError("Data mismatch occurred during execution.");
         break;
      }
   }

   state.SetItemsProcessed(state.iterations() * count);
}
BENCHMARK(BM_RingBuffer_MultiThreaded)->ArgsProduct({{2, 8, 64, 512, 1024 * 10}, {1'000'000}})->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
