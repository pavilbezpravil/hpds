#include "Helpers.h"

#include <random>
#include <ranges>

thread_local uint32_t tlRandPcgState;

uint32_t HashPcg(uint32_t& state) {
   state = state * 747796405u + 2891336453u;
   uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
   return (word >> 22u) ^ word;
}

uint32_t RandPcg() {
   return HashPcg(tlRandPcgState);
}

float RandFloat() {
   return float(RandPcg()) / float(UINT_MAX);
}

bool RandBool() {
   return RandPcg() > UINT_MAX / 2;
}

uint32_t RandUint(uint32_t minValue, uint32_t maxValue) {
   return minValue + uint32_t(float(maxValue - minValue) * RandFloat());
}

void BusyWaitForNanoseconds(int nanoseconds) {
   auto start = std::chrono::high_resolution_clock::now();
   while (true) {
      auto now = std::chrono::high_resolution_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start);
      if (elapsed.count() >= nanoseconds) {
         break;
      }
   }
}

void EmulateWork(int iterations) {
   volatile double result = 0.0;
   for (int i = 0; i < iterations; ++i) {
      result += std::sin(i) * std::cos(i);
   }
}

std::vector<uint8_t> GenerateRandomBytes(int count, uint8_t minValue, uint8_t maxValue) {
   std::random_device rd;
   std::mt19937 gen(rd());
   std::uniform_int_distribution<uint32_t> dis(minValue, maxValue);

   auto randomNumbersView = std::views::iota(0, count)
      | std::views::transform([&](int) { return dis(gen); });
   return { randomNumbersView.begin(), randomNumbersView.end() };
}

std::vector<int> GenerateRandomIntegers(int count, int minValue, int maxValue) {
   std::random_device rd;
   std::mt19937 gen(rd());
   std::uniform_int_distribution dis(minValue, maxValue);

   auto randomNumbersView = std::views::iota(0, count)
      | std::views::transform([&](int) { return dis(gen); });
   return { randomNumbersView.begin(), randomNumbersView.end() };
}

std::vector<float> GenerateRandomDoubles(int count, float minValue, float maxValue) {
   std::random_device rd;
   std::mt19937 gen(rd());
   std::uniform_real_distribution dis(minValue, maxValue);

   auto randomNumbersView = std::views::iota(0, count)
      | std::views::transform([&](int) { return dis(gen); });
   return { randomNumbersView.begin(), randomNumbersView.end() };
}

std::vector<int> GenerateSequentialVector(int N) {
   auto range = std::views::iota(0, N);
   std::vector<int> result(range.begin(), range.end());
   return result;
}
