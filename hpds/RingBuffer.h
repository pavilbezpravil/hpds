#pragma once
#include <vector>
#include <optional>

#define ALIGN_CACHE_LINE alignas(std::hardware_destructive_interference_size)

template<typename T>
class RingBuffer {
public:
   RingBuffer(int capacity) : capacity(capacity) {
      buffer.resize(capacity);
   }

   bool Push(const T& data) {
      int curTail = tail.load(std::memory_order::relaxed);
      int nextTail = Increment(curTail);

      if (nextTail == headCached) {
         headCached = head.load(std::memory_order::acquire);
         if (nextTail == headCached) {
            return false;
         }
      }

      buffer[curTail] = data;
      tail.store(nextTail, std::memory_order::release);

      return true;

   }

   std::optional<T> Pop() {
      int curHead = head.load(std::memory_order::relaxed);
      if (curHead == tailCached) {
         tailCached = tail.load(std::memory_order::acquire);
         if (curHead == tailCached) {
            return {};
         }
      }

      head.store(Increment(curHead), std::memory_order::release);
      return std::move(buffer[curHead]);
   }

   T PopWait() {
      while (true) {
         if (auto data = Pop()) {
            return data.value();
         }
      }
   }

   bool WasEmpty() const {
      return tail == head;
   }

   bool WasFull() const {
      return Increment(tail) == head;
   }

private:
   std::vector<T> buffer;
   int capacity;

   // false sharing decrease perf by ~2
   ALIGN_CACHE_LINE std::atomic<int> head = 0;
   ALIGN_CACHE_LINE std::atomic<int> tail = 0;

   // Idea from https://rigtorp.se/ringbuffer/
   // Without this optimization throughput 107 M/s, with 538 M/s
   ALIGN_CACHE_LINE int headCached = 0;
   ALIGN_CACHE_LINE int tailCached = 0;

   int Increment(int val) const {
      int nextVal = val + 1;
      return nextVal == capacity ? 0 : nextVal;
   }
};

#include "Helpers.h"

inline bool RingBufferMultiThreadTest(int ringBufferSize, int count, bool emulateWork = false) {
   RingBuffer<int> rb{ ringBufferSize };
   int nextExpected = 0;

   std::thread writer{
   [&]
   {
      int data = 0;
      while (data < count) {
         while (!rb.Push(data));
         ++data;

         if (emulateWork) {
            EmulateWork(4);
         }
      }
   } };

   while (nextExpected < count) {
      int data = rb.PopWait();

      if (data != nextExpected) {
         break;
      }
      ++nextExpected;

      if (emulateWork) {
         EmulateWork(4 * 2);
      }
   }

   writer.join();

   return nextExpected == count;
}
