#pragma once
#include <chrono>

uint32_t HashPcg(uint32_t& state);
uint32_t RandPcg();

/*
----------------------------------------------------------------------------
Benchmark                                  Time             CPU   Iterations
----------------------------------------------------------------------------
BM_BusyWaitForNanoseconds/1              101 ns          103 ns      7466667
BM_BusyWaitForNanoseconds/8              101 ns          100 ns      7466667
BM_BusyWaitForNanoseconds/64             101 ns          100 ns      6400000
BM_BusyWaitForNanoseconds/512            604 ns          614 ns      1120000
BM_BusyWaitForNanoseconds/4096          4110 ns         4081 ns       172308
 */
// Cant emulate less that 100 ns on my CPU
void BusyWaitForNanoseconds(int nanoseconds = 100);

/*
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
BM_EmulateWork/1             3.00 ns         2.98 ns    235789474
BM_EmulateWork/2             12.4 ns         12.0 ns     56000000
BM_EmulateWork/4             30.9 ns         31.4 ns     22400000
BM_EmulateWork/8             67.1 ns         66.3 ns      8960000
BM_EmulateWork/16             143 ns          144 ns      4977778
BM_EmulateWork/32             291 ns          292 ns      2357895
 */
// 1 - 16 useful range, than use BusyWaitForNanoseconds
void EmulateWork(int iterations = 4);

std::vector<uint8_t> GenerateRandomBytes(int count, uint8_t minValue = 0, uint8_t maxValue = 0xff);
std::vector<int> GenerateRandomIntegers(int count, int minValue = 0, int maxValue = INT_MAX);
std::vector<float> GenerateRandomDoubles(int count, float minValue = 0.f, float maxValue = 1000.f);

std::vector<int> GenerateSequentialVector(int N);
