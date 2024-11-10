#pragma once
#include <vector>
#include <cassert>

class IndexPool {
public:
   IndexPool(int capacity) : capacity(capacity) {
      freeIndices.reserve(capacity);
   }

   int Allocate() {
      if (!freeIndices.empty()) {
         int index = freeIndices.back();
         freeIndices.pop_back();
         return index;
      }
      if (nextIndex < capacity) {
         return nextIndex++;
      }
      assert(false && "No more indices available in the pool.");
      return -1;
   }

   void Free(int index) {
      assert(index < nextIndex && "Invalid index to free.");
      freeIndices.push_back(index);
   }

   int Available() const {
      return capacity - nextIndex + (int)freeIndices.size();
   }
private:
   int capacity;
   int nextIndex = 0;
   std::vector<int> freeIndices;
};
