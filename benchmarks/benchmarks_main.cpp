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

Looks that better use mutex if you dont have proof and special knowledge why your custom lock will be better

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

/*
On sorted data 10 times faster.

----------------------------------------------------------------
Benchmark                      Time             CPU   Iterations
----------------------------------------------------------------
BM_BranchPrediction/0       3.30 ms         3.30 ms          213 unsorted
BM_BranchPrediction/1      0.387 ms        0.393 ms         1867 sorted
 */
static void BM_BranchPrediction(benchmark::State& state) {
   auto numbers = GenerateRandomIntegers(1'000'000);

   bool doSort = (int)state.range(0) == 1;
   if (doSort) {
      std::ranges::sort(numbers);
   }

   state.SetLabel(doSort ? "sorted" : "unsorted");

   for (auto _ : state) {
      int threshold = INT_MAX / 2;

      volatile int acc = 0;
      for (int number : numbers) {
         if (number < threshold) {
            ++acc;
         }
      }
   }
}
BENCHMARK(BM_BranchPrediction)->Arg(0)->Arg(1)->Unit(benchmark::kMillisecond);

int GetMB(int nMB) {
   return (1 << 20) * nMB;
}

static void BM_MemoryAccess_Offset(benchmark::State& state) {
   int nNumbers = GetMB(256);
   auto numbers = GenerateRandomIntegers(nNumbers);

   int offset = (int)state.range(0) / 4; // offset in int's
   int count = GetMB(1);

   bool isRandom = offset == 0;

   if (isRandom) {
      for (auto _ : state) {
         int acc = 0;

         for (int i = 0; i < count; ++i) {
            uint32_t iElement = RandPcg() & (nNumbers - 1); // % nNumbers in case of pow of 2
            acc += numbers[iElement];
         }

         benchmark::DoNotOptimize(acc);
      }

      state.SetLabel("random access");
   } else { // offset access
      for (auto _ : state) {
         int acc = 0;

         int iElement = 0;
         for (int i = 0; i < count; ++i) {
            iElement = (iElement + offset) & (nNumbers - 1); // % nNumbers in case of pow of 2
            acc += numbers[iElement];
         }

         benchmark::DoNotOptimize(acc);
      }
   }

   state.SetItemsProcessed(state.iterations() * count * sizeof(int));
}
/*
Max mem throughput 8.7G/s
One thread: worst 326M/s (20 slower), cache line 1.7G/s
16 threads: worst 53M/s (164 slower), cache line 126M/s

# One thread.
Random offset is worst way to access memory 20 times slower that ideal with 4 bytes
Cache line offset is 5 times bad, two cache lines 10 times bad.

---------------------------------------------------------------------------------------
Benchmark                             Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------------------
BM_MemoryAccess_Offset/0           12.9 ms         12.8 ms           56 items_per_second=326.791M/s random access
BM_MemoryAccess_Offset/4          0.475 ms        0.481 ms         1493 items_per_second=8.71248G/s
BM_MemoryAccess_Offset/8          0.481 ms        0.487 ms         1445 items_per_second=8.61976G/s
BM_MemoryAccess_Offset/16         0.720 ms        0.715 ms          896 items_per_second=5.8663G/s
BM_MemoryAccess_Offset/32          1.18 ms         1.17 ms          560 items_per_second=3.57914G/s
BM_MemoryAccess_Offset/64          2.37 ms         2.35 ms          299 items_per_second=1.7836G/s
BM_MemoryAccess_Offset/128         4.98 ms         4.96 ms          145 items_per_second=846.155M/s
BM_MemoryAccess_Offset/256         6.10 ms         6.09 ms          100 items_per_second=688.296M/s
BM_MemoryAccess_Offset/512         6.84 ms         6.77 ms           90 items_per_second=619.466M/s
BM_MemoryAccess_Offset/1024        7.11 ms         7.11 ms          112 items_per_second=589.505M/s
BM_MemoryAccess_Offset/2048        7.09 ms         7.12 ms           90 items_per_second=589.249M/s
BM_MemoryAccess_Offset/4096        6.24 ms         6.28 ms          112 items_per_second=668.106M/s
BM_MemoryAccess_Offset/8192        6.82 ms         6.84 ms          112 items_per_second=613.567M/s
BM_MemoryAccess_Offset/16384       9.26 ms         9.17 ms           75 items_per_second=457.56M/s
BM_MemoryAccess_Offset/32768       10.9 ms         11.0 ms           64 items_per_second=381.775M/s
BM_MemoryAccess_Offset/65536       11.5 ms         11.4 ms           56 items_per_second=366.644M/s
 */
BENCHMARK(BM_MemoryAccess_Offset)->Arg(0)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_MemoryAccess_Offset)->RangeMultiplier(2)->Range(4, 1024 * 64)->Unit(benchmark::kMillisecond);
/*
# All logical threads dramatically decrease perf.
Random access - 50 times
Cache line access - 18 times
2 cache line access - 21 times

--------------------------------------------------------------------------------------------------
Benchmark                                        Time             CPU   Iterations UserCounters...
--------------------------------------------------------------------------------------------------
BM_MemoryAccess_Offset/0/threads:16           82.9 ms         78.1 ms           16 items_per_second=53.6871M/s random access
BM_MemoryAccess_Offset/4/threads:16           1.71 ms         1.72 ms          400 items_per_second=2.44032G/s
BM_MemoryAccess_Offset/8/threads:16           3.79 ms         4.00 ms          160 items_per_second=1.04755G/s
BM_MemoryAccess_Offset/16/threads:16          7.83 ms         10.4 ms           48 items_per_second=402.653M/s
BM_MemoryAccess_Offset/32/threads:16          15.9 ms         15.0 ms           48 items_per_second=280.107M/s
BM_MemoryAccess_Offset/64/threads:16          31.5 ms         33.2 ms           32 items_per_second=126.323M/s
BM_MemoryAccess_Offset/128/threads:16         37.4 ms         35.2 ms           32 items_per_second=119.305M/s
BM_MemoryAccess_Offset/256/threads:16         36.4 ms         43.0 ms           16 items_per_second=97.6129M/s
BM_MemoryAccess_Offset/512/threads:16         43.4 ms         31.2 ms           16 items_per_second=134.218M/s
BM_MemoryAccess_Offset/1024/threads:16        35.1 ms         34.2 ms           16 items_per_second=122.713M/s
BM_MemoryAccess_Offset/2048/threads:16        34.2 ms         38.1 ms           32 items_per_second=110.127M/s
BM_MemoryAccess_Offset/4096/threads:16        45.9 ms         46.9 ms           16 items_per_second=89.4785M/s
BM_MemoryAccess_Offset/8192/threads:16        52.4 ms         59.6 ms           16 items_per_second=70.4093M/s
BM_MemoryAccess_Offset/16384/threads:16       58.4 ms         58.6 ms           16 items_per_second=71.5828M/s
BM_MemoryAccess_Offset/32768/threads:16       67.4 ms         68.4 ms           16 items_per_second=61.3567M/s
BM_MemoryAccess_Offset/65536/threads:16       55.0 ms         54.7 ms           16 items_per_second=76.6958M/s
 */
BENCHMARK(BM_MemoryAccess_Offset)->Arg(0)->Unit(benchmark::kMillisecond)->ThreadPerCpu();
BENCHMARK(BM_MemoryAccess_Offset)->RangeMultiplier(2)->Range(4, 1024 * 64)->Unit(benchmark::kMillisecond)->ThreadPerCpu();

BENCHMARK_MAIN();
