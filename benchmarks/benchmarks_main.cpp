#include <benchmark/benchmark.h>

#include "Helpers.h"
#include "RingBuffer.h"
#include "SpinLock.h"

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
BENCHMARK(BM_RingBuffer_MultiThreaded)->ArgsProduct({{2, 8, 16, 64, 256, 1024, 1024 * 10}, {1'000'000}})->Unit(benchmark::kMillisecond);

// Fully utilize CPU. Real time == CPU time
static void BM_QSort(benchmark::State& state) {
   for (auto _ : state) {
      state.PauseTiming();
      auto random = GenerateRandomIntegers((int)state.range(0));
      state.ResumeTiming();

      std::ranges::sort(random);
   }
}
BENCHMARK(BM_QSort)->Range(1, 10'000'000)->Unit(benchmark::kMillisecond);

void ThreadsJoin(std::vector<std::thread>& threads) {
   for (auto& thread : threads) {
      thread.join();
   }
}

void ThreadCooperativeStartSpin(std::atomic<int>& letsGo, int nThreads) {
   ++letsGo;
   while (letsGo != nThreads);
}

template <typename SpinLockType>
static void BM_SpinLock(benchmark::State& state) {
   int count = 1'000'000;
   int nThreads = (int)state.range(0);

   SpinLockType spinLock;

   for (auto _ : state) {
      state.PauseTiming();

      std::atomic<int> letsGo = 0;
      std::vector<std::thread> threads;

      auto Task = [&]
      {
         ThreadCooperativeStartSpin(letsGo, nThreads);

         for (int i = 0; i < count; ++i) {
            std::lock_guard guard{ spinLock };
         }
      };

      for (int i = 0; i < nThreads - 1; ++i) {
         threads.emplace_back([&]{
            Task();
         });
      }

      state.ResumeTiming();

      Task();

      ThreadsJoin(threads);
   }
}
/*
Mutex in most cases better.
When 2-4 thread mutex win. 1, 4-16 SpinLock win.

----------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations
----------------------------------------------------------------------
BM_SpinLock<std::mutex>/1         12.0 ms         12.2 ms           64
BM_SpinLock<SpinLock>/1           5.25 ms         5.31 ms          100
BM_SpinLock<std::mutex>/2         32.8 ms         30.2 ms           30
BM_SpinLock<SpinLock>/2           32.0 ms         28.1 ms           20
BM_SpinLock<std::mutex>/4          112 ms          112 ms            6
BM_SpinLock<SpinLock>/4            112 ms          109 ms            5
BM_SpinLock<std::mutex>/8          395 ms          391 ms            2
BM_SpinLock<SpinLock>/8            308 ms          289 ms            2
BM_SpinLock<std::mutex>/16         1545 ms         1500 ms           1
BM_SpinLock<SpinLock>/16           1162 ms         1156 ms           1
 */
BENCHMARK_TEMPLATE(BM_SpinLock, std::mutex)->RangeMultiplier(2)->Range(1, 16)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SpinLock, SpinLockTAS)->RangeMultiplier(2)->Range(1, 16)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SpinLock, SpinLockTTAS)->RangeMultiplier(2)->Range(1, 16)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SpinLock, SpinLock)->RangeMultiplier(2)->Range(1, 16)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
